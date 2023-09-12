#include "json_reader.h"
#include "json_builder.h"
#include "serialization.h"

#include <fstream>


void JsonReader::ProcessQueries() { //сохнаряем все запросы в мапу и сразу их обрабатываем в правильном порядке
	json::Document all_requests_queue = json::Load(in_);

	if (all_requests_queue.GetRoot().AsDict().count("base_requests")){
		ParseAndProcessBaseRequests(all_requests_queue.GetRoot().AsDict().at("base_requests").AsArray());
	}

	if (all_requests_queue.GetRoot().AsDict().count("render_settings")){
		const json::Dict& render_settings_map = all_requests_queue.GetRoot().AsDict().at("render_settings").AsDict();
		map_renderer_.SetRenderSettings(ParseRenderSettings(render_settings_map));
	}

	if (all_requests_queue.GetRoot().AsDict().count("routing_settings")){
		const json::Dict& routing_settings_map = all_requests_queue.GetRoot().AsDict().at("routing_settings").AsDict();
		transport_router_.SetRoutingSettings(ParseRoutingSettings(routing_settings_map));
		transport_router_.PrepareRouter();
	}

	if (all_requests_queue.GetRoot().AsDict().count("stat_requests"))
	{
		const std::vector<json::Node>& stat_requests_queue_nodes = all_requests_queue.GetRoot().AsDict().at("stat_requests").AsArray();
		//в одном цикле и парсим, и обрабатываем
		if (!stat_requests_queue_nodes.empty()) {
			StatRequest_ParseAndProcessNode(stat_requests_queue_nodes);
		}
	}
}

void JsonReader::make_base()
{
	json::Document all_requests_queue = json::Load(in_);

	if (all_requests_queue.GetRoot().AsDict().count("base_requests")) {
		ParseAndProcessBaseRequests(all_requests_queue.GetRoot().AsDict().at("base_requests").AsArray());
	}

	SerializationSettings serialization_settings = ParseSerializationSettings(all_requests_queue.GetRoot().AsDict().at("serialization_settings").AsDict());
	std::ofstream serialized_file_out(serialization_settings.file_name, std::ios::binary);

	const json::Dict& render_settings_map = all_requests_queue.GetRoot().AsDict().at("render_settings").AsDict();
	RenderSettings render_settings = ParseRenderSettings(render_settings_map);

	const json::Dict& routing_settings_map = all_requests_queue.GetRoot().AsDict().at("routing_settings").AsDict();
	RoutingSettings routing_settings = ParseRoutingSettings(routing_settings_map);

	SerializeAll(catalogue_, render_settings, routing_settings, serialized_file_out);
}

void JsonReader::process_requests()
{
	json::Document all_requests_queue = json::Load(in_);
	SerializationSettings serialization_settings = ParseSerializationSettings(all_requests_queue.GetRoot().AsDict().at("serialization_settings").AsDict());
	std::ifstream serialized_file_in(serialization_settings.file_name, std::ios::binary);
	if (!serialized_file_in) {
		throw std::invalid_argument("Failed to open serialized file");
	}
	RenderSettings render_settings;
	RoutingSettings routing_settings;
	DeserializeAllInPlace(serialized_file_in, catalogue_, render_settings, routing_settings);
	map_renderer_.SetRenderSettings(render_settings);
	transport_router_.SetRoutingSettings(routing_settings);
	transport_router_.PrepareRouter();
	if (all_requests_queue.GetRoot().AsDict().count("stat_requests"))
	{
		const std::vector<json::Node>& stat_requests_queue_nodes = all_requests_queue.GetRoot().AsDict().at("stat_requests").AsArray();
		//в одном цикле и парсим, и обрабатываем
		if (!stat_requests_queue_nodes.empty()) {
			StatRequest_ParseAndProcessNode(stat_requests_queue_nodes);
		}
	}
}

void JsonReader::ParseAndProcessBaseRequests(const std::vector<json::Node>& base_requests_queue_nodes) {
	//формируем очереди запросов на добавление остановок и маршрутов
	for (const json::Node& request_node : base_requests_queue_nodes) {
		const json::Dict& request_node_map = request_node.AsDict();
		if (request_node_map.at("type").AsString() == "Bus") {
			base_requests_buses_queue_.push_back(&request_node_map);
		}
		else if (request_node_map.at("type").AsString() == "Stop") {
			base_requests_stops_queue_.push_back(&request_node_map);
		}
		else { throw std::invalid_argument("Base request type not recognized"); }
	}

	//формируем структуры остановок и добавляем их в каталог
	for (Stop& stop : BaseRequest_FormStopStructs()) {
		catalogue_.AddStop(std::move(stop));
	}

	//задаем расстояние между остановками
	for (const auto& [stop1, stop2, dist] : stops_to_distance_) {
		catalogue_.SetDistanceBetweenStops(catalogue_.FindStop(stop1), catalogue_.FindStop(stop2), dist);
	}

	//формируем структуры маршрутов и добавляем в каталог
	for (Bus& bus : BaseRequest_FormBusStructs()) {
		catalogue_.AddBus(std::move(bus));
	}
}

std::vector<Stop> JsonReader::BaseRequest_FormStopStructs() {
	std::vector<Stop> output;
	output.reserve(base_requests_stops_queue_.size());
	for (const json::Dict* stop_node_map : base_requests_stops_queue_) {
		output.push_back(BaseRequest_ParseStopNode(stop_node_map));
		BaseRequest_ParseDistanceToOtherStops(stop_node_map);
	}
	return output;
}

Stop JsonReader::BaseRequest_ParseStopNode(const json::Dict* stop_node_map) {
	Stop output;
	output.name = stop_node_map->at("name").AsString();
	output.coordinates.lat = stop_node_map->at("latitude").AsDouble();
	output.coordinates.lng = stop_node_map->at("longitude").AsDouble();
	return output;
}

void JsonReader::BaseRequest_ParseDistanceToOtherStops(const json::Dict* stop_node_map) {
	const std::string_view cur_stop_name = stop_node_map->at("name").AsString();
	for (const auto& [second_stop_name, Node_dist] : stop_node_map->at("road_distances").AsDict()) {
		stops_to_distance_.push_back({ cur_stop_name, second_stop_name, Node_dist.AsInt() });
	}
}

std::vector<Bus> JsonReader::BaseRequest_FormBusStructs() {
	std::vector<Bus> output;
	output.reserve(base_requests_buses_queue_.size());
	for (const json::Dict* bus_node_map : base_requests_buses_queue_) {
		output.push_back(BaseRequest_ParseBusNode(bus_node_map));
	}
	return output;
}

Bus JsonReader::BaseRequest_ParseBusNode(const json::Dict* bus_node_map) {
	Bus output;
	output.name = bus_node_map->at("name").AsString();
	std::unordered_set<std::string_view> unique_stop_names;
	for (const json::Node& stop_name_node : bus_node_map->at("stops").AsArray()) { //полная запись
		std::string_view stop_name = stop_name_node.AsString();
		output.stops.push_back(catalogue_.FindStop(stop_name));
		unique_stop_names.emplace(stop_name);
	}
	if (!bus_node_map->at("is_roundtrip").AsBool()) { //половинчатая запись
		output.stops.insert(output.stops.end(), output.stops.rbegin() + 1, output.stops.rend());
	}
	output.is_roundtrip = bus_node_map->at("is_roundtrip").AsBool();

	output.unique_stops_number = static_cast<unsigned int>(unique_stop_names.size());

	double route_length_geo = ComputeGeoRouteLength(output); //для извилистости вычисляем географическое расстояние
	ComputeActualRouteLength(output); //сразу записываем значение в output
	output.curvature = output.route_length / route_length_geo;
	return output;
}

double JsonReader::ComputeGeoRouteLength(const Bus& current_bus) {
	double route_length_geo = 0; //для извилистости вычисляем географическое расстояние
	for (std::vector<const Stop*>::const_iterator it0 = current_bus.stops.begin(); it0 != current_bus.stops.end() - 1; ++it0) {
		std::vector<const Stop*>::const_iterator it1 = it0 + 1;
		route_length_geo += geo::ComputeDistance((**it0).coordinates, (**it1).coordinates);
	}
	return route_length_geo;
}

void JsonReader::ComputeActualRouteLength(Bus& current_bus) {
	for (std::vector<const Stop*>::iterator it0 = current_bus.stops.begin(); it0 != current_bus.stops.end() - 1; ++it0) { //вычисляем маршрутное расстояние
		std::vector<const Stop*>::iterator it1 = it0 + 1;

		current_bus.route_length += catalogue_.ComputeActualDistanceBetweenNearbyStops(it0, it1);
	}
}

void JsonReader::StatRequest_ParseAndProcessNode(const std::vector<json::Node>& stat_requests_queue_nodes) {
	using namespace std::literals;

	json::Array results;
	for (const json::Node& request_node : stat_requests_queue_nodes) {
		const json::Dict& request_node_map = request_node.AsDict();

		if (request_node_map.at("type"s).AsString() == "Stop"s) { //Stop request
			const Stop* found_stop = catalogue_.FindStop(request_node_map.at("name"s).AsString());
			if (!found_stop) {
				results.push_back(json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
					.EndDict()
					.Build()
				);
			}
			else {
				std::set<std::string_view> found_stops = catalogue_.GetBusesOnStop(request_node_map.at("name"s).AsString());
				json::Array found_stops_nodes;
				found_stops_nodes.reserve(found_stops.size());
				for (const std::string_view& stop_name_sv : found_stops) {
					found_stops_nodes.push_back(json::Node(std::string(stop_name_sv)));
				}

				results.push_back(json::Builder{}
					.StartDict()
						.Key("buses"s).Value(std::move(found_stops_nodes))
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
					.EndDict()
					.Build()
				);
			}
		}
		else if (request_node_map.at("type"s).AsString() == "Bus"s) {	//Bus request
			const Bus* found_bus = catalogue_.FindBus(request_node_map.at("name"s).AsString());
			if (!found_bus) {
				results.push_back(json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
					.EndDict()
					.Build()
				);
			}
			else {
				results.push_back(json::Builder{}
					.StartDict()
						.Key("curvature"s).Value(found_bus->curvature)
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
						.Key("route_length"s).Value(found_bus->route_length)
						.Key("stop_count"s).Value(static_cast<int>(found_bus->stops.size()))
						.Key("unique_stop_count"s).Value(static_cast<int>(found_bus->unique_stops_number))
					.EndDict()
					.Build()
				);
			}
		}
		else if (request_node_map.at("type"s).AsString() == "Map"s) {
			std::ostringstream string_stream;
			map_renderer_.DrawMap(string_stream);
			results.push_back(json::Builder{}
				.StartDict()
					.Key("map"s).Value(std::move(string_stream.str()))
					.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
				.EndDict()
				.Build()
			);
		}
		else if (request_node_map.at("type"s).AsString() == "Route"s) {
			const Stop* stop_from = catalogue_.FindStop(request_node_map.at("from"s).AsString());
			const Stop* stop_to = catalogue_.FindStop(request_node_map.at("to"s).AsString());
			//проверка на nullptr?
			std::optional<graph::Router<double>::RouteInfo> route = transport_router_.BuildRoute(stop_from->VertexId, stop_to->VertexId);
			if (!route) {
				results.push_back(json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
					.EndDict()
					.Build()
				);
			}
			else {
				//создаем array из активностей
				json::Builder route_items_array_builder{};
				route_items_array_builder.StartArray();
				for (size_t edge_id : route.value().edges) { //т.к. в массу грани включено ожидание, перед каждой добавляем wait
					const graph::Edge<double>& edge = transport_router_.GetEdge(edge_id);
					route_items_array_builder.StartDict()
						//wait
						.Key("type"s).Value("Wait"s)
						.Key("stop_name"s).Value(catalogue_.GetStopByID(edge.from)->name)
						.Key("time"s).Value(transport_router_.GetWaitTime())
					.EndDict()
						//bus
					.StartDict()
						.Key("type"s).Value("Bus"s)
						.Key("bus"s).Value(std::string(edge.bus_name))
						.Key("span_count"s).Value(edge.span_count)
						.Key("time"s).Value(edge.weight - transport_router_.GetWaitTime())//в массу грани включено ожидание,
					.EndDict();						//поэтому для получения времени движения на одном автобусе надо вычесть время ожидания
				}
				route_items_array_builder.EndArray();
				//формируем полный ответ
				results.push_back(json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(request_node_map.at("id"s).AsInt())
						.Key("total_time"s).Value(route.value().weight)
						.Key("items"s).Value(route_items_array_builder.Build())
					.EndDict()
					.Build()
				);
			}
		}
		else { throw std::invalid_argument("Stat request type not recognized: "s + request_node_map.at("type"s).AsString()); }
	}

	json::PrintNode(results, out_);
}

svg::Color JsonReader::ParseColorNode(const json::Node& color_node) {
	svg::Color output;
	if (color_node.IsString()) {
		output = color_node.AsString();
	}
	else if (color_node.IsArray()) {
		const json::Array& color_number_nodes = color_node.AsArray();
		if (color_number_nodes.size() == 3) {
			output = svg::Rgb(
				color_number_nodes[0].AsInt(),
				color_number_nodes[1].AsInt(),
				color_number_nodes[2].AsInt());
		}
		else {
			output = svg::Rgba(
				color_number_nodes[0].AsInt(),
				color_number_nodes[1].AsInt(),
				color_number_nodes[2].AsInt(),
				color_number_nodes[3].AsDouble());
		}
	}
	return output;
}

RenderSettings JsonReader::ParseRenderSettings(const json::Dict& render_settings_map) {
	using namespace std::literals;
	RenderSettings output;
	output.width = render_settings_map.at("width"s).AsDouble();
	output.height = render_settings_map.at("height"s).AsDouble();
	output.padding = render_settings_map.at("padding"s).AsDouble();
	output.stop_radius = render_settings_map.at("stop_radius"s).AsDouble();
	output.line_width = render_settings_map.at("line_width"s).AsDouble();
	output.bus_label_font_size = render_settings_map.at("bus_label_font_size"s).AsInt();
	output.bus_label_offset = { render_settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(), render_settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble() };
	output.stop_label_font_size = render_settings_map.at("stop_label_font_size"s).AsInt();
	output.stop_label_offset = { render_settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(), render_settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble() };
	output.underlayer_color = ParseColorNode(render_settings_map.at("underlayer_color"s));
	output.underlayer_width = render_settings_map.at("underlayer_width"s).AsDouble();

	for (const json::Node& color_node : render_settings_map.at("color_palette"s).AsArray()) {
		output.color_palette.push_back(ParseColorNode(color_node));
	}

	return output;
}

RoutingSettings JsonReader::ParseRoutingSettings(const json::Dict& routing_settings_map) {
	return {
	routing_settings_map.at("bus_wait_time").AsDouble(),
	routing_settings_map.at("bus_velocity").AsDouble()
	};
}

SerializationSettings JsonReader::ParseSerializationSettings(const json::Dict& serialization_settings_map)
{
	return { serialization_settings_map.at("file").AsString() };
}
