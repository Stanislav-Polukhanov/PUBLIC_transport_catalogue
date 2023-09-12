#include "serialization.h"

TransportCataloguePB::TransportCatalogue SerializeCatalogue(const TransportCatalogue& catalogue)
{
	//собираем структуру PB всей базы данных каталога
	TransportCataloguePB::TransportCatalogue catalogue_database_pb;

	//остановки
	for (const Stop& stop : catalogue.GetStopsDeque()) {
		//собираем структуру остановки для PB
		TransportCataloguePB::Stop stop_pb;
		//имя
		stop_pb.set_name(stop.name);
		//id
		stop_pb.set_vertex_id(stop.VertexId);
		//координаты
		TransportCataloguePB::Coordinates coordinates_pb;
		coordinates_pb.set_lat(stop.coordinates.lat);
		coordinates_pb.set_lng(stop.coordinates.lng);
		*stop_pb.mutable_coordinates() = std::move(coordinates_pb);
		//остановка готова - суем в датабазу
		*catalogue_database_pb.add_stops() = std::move(stop_pb);
	}

	//расстояния
	for (auto [stop_ptr_pair, distance] : catalogue.GetDistancesUmap()) {
		//собираем структуру дистанции для PB
		TransportCataloguePB::DistanceBetweenStops distance_pb;
		distance_pb.set_stop1_id(stop_ptr_pair.first->VertexId);
		distance_pb.set_stop2_id(stop_ptr_pair.second->VertexId);
		distance_pb.set_distance(distance);
		//суем ее в датабазу
		*catalogue_database_pb.add_distances_between_stops() = std::move(distance_pb);
	}

	//маршруты
	for (auto [bus_name, bus_ptr] : catalogue.GetBusesMap()) {
		//собираем структуру маршрута для PB
		TransportCataloguePB::Bus bus_pb;
		//имя
		bus_pb.set_name(std::move(std::string(bus_name)));
		//массив ID остановок
		for (const Stop* stop : bus_ptr->stops) {
			bus_pb.add_stop_id_pb_array(stop->VertexId);
		}
		//кол-во уникальных остановок
		bus_pb.set_unique_stops_number(bus_ptr->unique_stops_number);
		//длина маршрута
		bus_pb.set_route_length(bus_ptr->route_length);
		//кривизна
		bus_pb.set_curvature(bus_ptr->curvature);
		//флаг кольцевого маршрута
		bus_pb.set_is_roundtrip(bus_ptr->is_roundtrip);
		//маршрут готов - суем в датабазу
		*catalogue_database_pb.add_buses() = std::move(bus_pb);
	}

	//БД готова
	return catalogue_database_pb;
}

TransportCataloguePB::Color SerializeColor(const svg::Color& color) {
	TransportCataloguePB::Color underlayer_color_pb;

	if (std::holds_alternative<std::monostate>(color)) {
		underlayer_color_pb.set_text("none");
	}
	else if (std::holds_alternative<std::string>(color)) {
		underlayer_color_pb.set_text(std::get<std::string>(color));
	}
	else if (std::holds_alternative<svg::Rgb>(color)) {
		const svg::Rgb& rgb = std::get<svg::Rgb>(color);
		underlayer_color_pb.set_text("rgb");
		underlayer_color_pb.set_r(rgb.red);
		underlayer_color_pb.set_g(rgb.green);
		underlayer_color_pb.set_b(rgb.blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		const svg::Rgba& rgba = std::get<svg::Rgba>(color);
		underlayer_color_pb.set_text("rgba");
		underlayer_color_pb.set_r(rgba.red);
		underlayer_color_pb.set_g(rgba.green);
		underlayer_color_pb.set_b(rgba.blue);
		underlayer_color_pb.set_a(rgba.opacity);
	}
	return underlayer_color_pb;
}

TransportCataloguePB::RenderSettings SerializeRenderSettings(const RenderSettings& render_settings)
{
	//создаем структуру настроек рендеринга PB
	TransportCataloguePB::RenderSettings render_settings_pb;
	//заполняем простые поля
	render_settings_pb.set_width(render_settings.width);
	render_settings_pb.set_height(render_settings.height);
	render_settings_pb.set_padding(render_settings.padding);
	render_settings_pb.set_line_width(render_settings.line_width);
	render_settings_pb.set_stop_radius(render_settings.stop_radius);
	render_settings_pb.set_bus_label_font_size(render_settings.bus_label_font_size);
	render_settings_pb.set_stop_label_font_size(render_settings.stop_label_font_size);
	render_settings_pb.set_underlayer_width(render_settings.underlayer_width);
	//bus_label_offset
	TransportCataloguePB::Point bus_label_offset_pb;
	bus_label_offset_pb.set_x(render_settings.bus_label_offset.x);
	bus_label_offset_pb.set_y(render_settings.bus_label_offset.y);
	*render_settings_pb.mutable_bus_label_offset() = std::move(bus_label_offset_pb);
	//stop_label_offset
	TransportCataloguePB::Point stop_label_offset_pb;
	stop_label_offset_pb.set_x(render_settings.stop_label_offset.x);
	stop_label_offset_pb.set_y(render_settings.stop_label_offset.y);
	*render_settings_pb.mutable_stop_label_offset() = std::move(stop_label_offset_pb);
	//underlayer_color
	TransportCataloguePB::Color underlayer_color_pb = SerializeColor(render_settings.underlayer_color);
	*render_settings_pb.mutable_underlayer_color() = std::move(underlayer_color_pb);
	//color_palette
	for (const svg::Color& color : render_settings.color_palette) {
		TransportCataloguePB::Color color_pb = SerializeColor(color);
		*render_settings_pb.add_color_palette() = std::move(color_pb);
	}
	//готово
	return render_settings_pb;
}

TransportCataloguePB::RoutingSettings SerializeRoutingSettings(const RoutingSettings& routing_settings)
{
	TransportCataloguePB::RoutingSettings routing_settings_pb;

	routing_settings_pb.set_bus_velocity(routing_settings.bus_velocity);
	routing_settings_pb.set_bus_wait_time(routing_settings.bus_wait_time);

	return routing_settings_pb;
}


void SerializeAll(
	const TransportCatalogue& catalogue, 
	const RenderSettings& render_settings, 
	const RoutingSettings& routing_settings, 
	std::ostream& out)
{
	TransportCataloguePB::AllInfo all_info;
	TransportCataloguePB::TransportCatalogue catalogue_pb = SerializeCatalogue(catalogue);
	TransportCataloguePB::RenderSettings render_settings_pb = SerializeRenderSettings(render_settings);
	TransportCataloguePB::RoutingSettings routing_settings_pb = SerializeRoutingSettings(routing_settings);
	*all_info.mutable_catalogue() = std::move(catalogue_pb);
	*all_info.mutable_render_settings() = std::move(render_settings_pb);
	*all_info.mutable_routing_settings() = std::move(routing_settings_pb);
	all_info.SerializePartialToOstream(&out);
}



void DeserializeCatalogueInPlace(const TransportCataloguePB::TransportCatalogue& catalogue_database_pb, TransportCatalogue& catalogue)
{
	//добавляем остановки
	for (int i = 0; i < catalogue_database_pb.stops_size(); ++i) {
		const TransportCataloguePB::Stop& stop_pb = catalogue_database_pb.stops(i);
		Stop stop;
		stop.name = stop_pb.name();
		//stop.VertexId = stop_pb.vertex_id(); //не нужно - сделается в catalogue.AddStop
		const TransportCataloguePB::Coordinates& coordinates_pb = stop_pb.coordinates();
		stop.coordinates = { coordinates_pb.lat(), coordinates_pb.lng() };
		catalogue.AddStop(std::move(stop));
	}
	//задаем расстояния
	for (int i = 0; i < catalogue_database_pb.distances_between_stops_size(); ++i) {
		const TransportCataloguePB::DistanceBetweenStops& distance_pb = catalogue_database_pb.distances_between_stops(i);
		const Stop* stop1 = catalogue.GetStopByID(distance_pb.stop1_id());
		const Stop* stop2 = catalogue.GetStopByID(distance_pb.stop2_id());
		catalogue.SetDistanceBetweenStops(stop1, stop2, distance_pb.distance());
	}
	//добавляем маршруты
	for (int i = 0; i < catalogue_database_pb.buses_size(); ++i) {
		const TransportCataloguePB::Bus bus_pb = catalogue_database_pb.buses(i);
		Bus bus;
		bus.name = bus_pb.name();
		bus.unique_stops_number = bus_pb.unique_stops_number();
		bus.route_length = bus_pb.route_length();
		bus.curvature = bus_pb.curvature();
		bus.is_roundtrip = bus_pb.is_roundtrip();
		for (int j = 0; j < bus_pb.stop_id_pb_array_size(); ++j) {
			bus.stops.reserve(bus_pb.stop_id_pb_array_size());
			bus.stops.push_back(catalogue.GetStopByID(bus_pb.stop_id_pb_array(j)));
		}
		catalogue.AddBus(std::move(bus));
	}
}

svg::Color DeserializeColor(const TransportCataloguePB::Color& color_pb) {
	std::string_view color_text = color_pb.text();
	if (color_text == "none") {
		return svg::NoneColor;
	}
	else if (color_text == "rgb") {
		return { svg::Rgb{static_cast<uint8_t>(color_pb.r()), static_cast<uint8_t>(color_pb.g()), static_cast<uint8_t>(color_pb.b())} };
	}
	else if (color_text == "rgba") {
		return { svg::Rgba{static_cast<uint8_t>(color_pb.r()), static_cast<uint8_t>(color_pb.g()), static_cast<uint8_t>(color_pb.b()), color_pb.a()} };
	}
	else {
		return { color_pb.text() };
	}
}

void DeserializeRenderSettingsInPlace(const TransportCataloguePB::RenderSettings& render_settings_pb, RenderSettings& render_settings)
{
	render_settings.width = render_settings_pb.width();
	render_settings.height = render_settings_pb.height();
	render_settings.padding = render_settings_pb.padding();
	render_settings.line_width = render_settings_pb.line_width();
	render_settings.stop_radius = render_settings_pb.stop_radius();
	render_settings.bus_label_font_size = render_settings_pb.bus_label_font_size();
	render_settings.stop_label_font_size = render_settings_pb.stop_label_font_size();
	render_settings.underlayer_width = render_settings_pb.underlayer_width();

	const TransportCataloguePB::Point& bus_label_offset_pb = render_settings_pb.bus_label_offset();
	render_settings.bus_label_offset = { bus_label_offset_pb.x(), bus_label_offset_pb.y()};
	const TransportCataloguePB::Point& stop_label_offset_pb = render_settings_pb.stop_label_offset();
	render_settings.stop_label_offset = { stop_label_offset_pb.x(), stop_label_offset_pb.y()};

	const TransportCataloguePB::Color& underlayer_color_pb = render_settings_pb.underlayer_color();
	render_settings.underlayer_color = DeserializeColor(underlayer_color_pb);
	for (int i = 0; i < render_settings_pb.color_palette_size(); ++i) {
		render_settings.color_palette.push_back(DeserializeColor(render_settings_pb.color_palette(i)));
	}
}

void DeserializeRoutingSettingsInPlace(const TransportCataloguePB::RoutingSettings& routing_settings_pb, RoutingSettings& routing_settings)
{
	routing_settings.bus_velocity = routing_settings_pb.bus_velocity();
	routing_settings.bus_wait_time = routing_settings_pb.bus_wait_time();
}

void DeserializeAllInPlace(std::istream& in, TransportCatalogue& catalogue, RenderSettings& render_settings, RoutingSettings& routing_settings)
{
	TransportCataloguePB::AllInfo all_info;
	all_info.ParseFromIstream(&in);
	DeserializeCatalogueInPlace(all_info.catalogue(), catalogue);
	DeserializeRenderSettingsInPlace(all_info.render_settings(), render_settings);
	DeserializeRoutingSettingsInPlace(all_info.routing_settings(), routing_settings);
}
