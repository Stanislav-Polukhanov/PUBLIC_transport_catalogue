#pragma once

#include "json.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"

#include <unordered_set>
#include <sstream>
#include <memory>

class JsonReader {
public:
	JsonReader(std::istream& in, std::ostream& out, TransportCatalogue& catalogue, MapRenderer& map_renderer, TransportRouter& transport_router)
		: in_(in)
		, out_(out)
		, catalogue_(catalogue)
		, map_renderer_(map_renderer)
		, transport_router_(transport_router)
	{}

	void ProcessQueries();

	void make_base();
	void process_requests();


private:
				/*======    STORAGE    ======*/
	std::istream& in_;
	std::ostream& out_;
	TransportCatalogue& catalogue_;
	MapRenderer& map_renderer_;
	TransportRouter& transport_router_;

	std::vector<const json::Dict*> base_requests_stops_queue_; //ссылки на мапы с запросами на добавление остановок (оргиналы хранятся в all_requests_queue)
	std::vector<std::tuple<const std::string_view, const std::string_view, unsigned int>> stops_to_distance_; //две остановки и расстояние между ними
	std::vector<const json::Dict*> base_requests_buses_queue_; //ссылки на мапы с запросами на добавление маршрутов (оргиналы хранятся в all_requests_queue)
	/*если будут проблемы по памяти, можно после обработки всех запросов очистить очереди*/

				/*======    BASE REQUESTS    ======*/
	void ParseAndProcessBaseRequests(const std::vector<json::Node>& base_requests_queue_nodes);

	//формирует структуры Stop и вектор для заполнения расстояний между остановками
	std::vector<Stop> BaseRequest_FormStopStructs();

	Stop BaseRequest_ParseStopNode(const json::Dict* stop_node_map);

	void BaseRequest_ParseDistanceToOtherStops(const json::Dict* stop_node_map);

	std::vector<Bus> BaseRequest_FormBusStructs();

	Bus BaseRequest_ParseBusNode(const json::Dict* bus_node_map);


				/*======    STAT REQUESTS    ======*/
	void StatRequest_ParseAndProcessNode(const std::vector<json::Node>& stat_requests_queue_nodes);


				/*======    SETTINGS    ======*/
	RenderSettings ParseRenderSettings(const json::Dict& render_settings_map);

	RoutingSettings ParseRoutingSettings(const json::Dict& routing_settings_map);

	SerializationSettings ParseSerializationSettings(const json::Dict& serialization_settings_map);


				/*======    OTHER    ======*/
	double ComputeGeoRouteLength(const Bus& current_bus);

	void ComputeActualRouteLength(Bus& current_bus);

	svg::Color ParseColorNode(const json::Node& color_node);
};