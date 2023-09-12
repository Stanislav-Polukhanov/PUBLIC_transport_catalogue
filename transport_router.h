#pragma once

#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <memory>

class TransportRouter {
public:
	TransportRouter(const TransportCatalogue& catalogue)
		: catalogue_(catalogue)
	{}

	void SetRoutingSettings(RoutingSettings settings);

	void PrepareRouter();

	std::optional<graph::Router<double>::RouteInfo> BuildRoute(graph::VertexId from, graph::VertexId to) const;

	double GetWaitTime() const;

	const graph::Edge<double>& GetEdge(size_t edge_id) const;



private:
	const TransportCatalogue& catalogue_;
	RoutingSettings routing_settings_;
	std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_u_ptr = nullptr;
	std::unique_ptr<graph::Router<double>> router_u_ptr = nullptr;



	//обрабатываем одно направление маршрута - кольцо обрабатывается целиком, некольцо - по половинке
	void Graph_CreateEdgesForSingleBusDirection(
		std::vector<const Stop*>::const_iterator stop_it_first, 
		std::vector<const Stop*>::const_iterator stop_it_last, //last != past_the_last !!!
		std::string_view bus_name);

	//возвращает заданную вручную дистанцию, а если таковой нет - географическую
	double ComputeActualDistanceBetweenNearbyStops(
		std::vector<const Stop*>::const_iterator stop_it0,
		std::vector<const Stop*>::const_iterator stop_it1
	) const;
};