#include "transport_router.h"

void TransportRouter::SetRoutingSettings(RoutingSettings settings){
	routing_settings_ = settings;
}

void TransportRouter::PrepareRouter() {
	//������������� �����
	graph_u_ptr = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue_.GetStopsUMap().size());

	//���������� ����� �������
	for (const auto& [bus_name, bus] : catalogue_.GetBusesMap()) {
		if (bus->is_roundtrip) {
			Graph_CreateEdgesForSingleBusDirection(bus->stops.begin(), bus->stops.end() - 1, bus->name);
		}
		else {
			auto it_middle = bus->stops.begin() + ((bus->stops.size() - 1) / 2);
			Graph_CreateEdgesForSingleBusDirection(bus->stops.begin(), it_middle, bus->name);
			Graph_CreateEdgesForSingleBusDirection(it_middle, bus->stops.end() - 1, bus->name);
		}
	}

	//������������� �������
	router_u_ptr = std::make_unique<graph::Router<double>>(*graph_u_ptr);
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(
	graph::VertexId from, graph::VertexId to) const
{
	return router_u_ptr->BuildRoute(from, to);
}

double TransportRouter::GetWaitTime() const {
	return routing_settings_.bus_wait_time;
}

const graph::Edge<double>& TransportRouter::GetEdge(size_t edge_id) const {
	return graph_u_ptr->GetEdge(edge_id);
}

void TransportRouter::Graph_CreateEdgesForSingleBusDirection( //��� �����, ����� ��������� ������� ���� �������������, � �� ��� ��� .end()
	std::vector<const Stop*>::const_iterator stop_it_first,
	std::vector<const Stop*>::const_iterator stop_it_last,
	std::string_view bus_name)
{				//���� �� ����� � ������ ��������... ����!
	using It = std::vector<const Stop*>::const_iterator;
	for (It stop_it0 = stop_it_first; stop_it0 != stop_it_last; ++stop_it0) {
		for (It stop_it1 = stop_it0 + 1; stop_it1 != stop_it_last + 1; ++stop_it1) {
			double distance = 0;
			int span_count = 0;
			for (It it0 = stop_it0; it0 != stop_it1; ++it0) { //�������� ���������� ����� ����������� ��������������� �������
				It it1 = it0 + 1;
				distance += ComputeActualDistanceBetweenNearbyStops(it0, it1);
				++span_count;
			}
			graph::Edge<double> edge{
				(*stop_it0)->VertexId,
				(*stop_it1)->VertexId,
				distance / (routing_settings_.bus_velocity * 1000.0 / 60.0) + routing_settings_.bus_wait_time,
				bus_name,
				span_count
			};
			graph_u_ptr->AddEdge(edge);
		}
	}
}

double TransportRouter::ComputeActualDistanceBetweenNearbyStops(
	std::vector<const Stop*>::const_iterator stop_it0,
	std::vector<const Stop*>::const_iterator stop_it1) const
{
	double distance = catalogue_.GetDistanceBetweenStops({ *stop_it0, *stop_it1 });
	if (!std::isnan(distance)) { //���� ����� ����� ��������������� ���������
		return distance;
	}
	else { //���� �� ����� ����� ���������������, ���� �������� ���������
		distance = catalogue_.GetDistanceBetweenStops({ *stop_it1, *stop_it0 });
		if (!std::isnan(distance)) {
			return distance;
		}
		else { //���� �� ����� �������, ������� ���������
			return geo::ComputeDistance((**stop_it0).coordinates, (**stop_it1).coordinates);
		}
	}
}