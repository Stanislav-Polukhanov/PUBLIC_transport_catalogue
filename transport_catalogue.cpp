#include "transport_catalogue.h"
#include "geo.h"

#include <iostream>
#include <algorithm>
#include <cmath>

using distances_umap = std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, PairDistanceHash>;


void TransportCatalogue::AddStop(Stop&& stop) {
	stop.VertexId = stops_.size();
	stops_.push_back(std::move(stop));
	stop_name_to_stop_.insert({ stops_.back().name, &stops_.back() });
}

void TransportCatalogue::AddBus(Bus&& bus) {
	busses_.push_back(std::move(bus));
	bus_name_to_bus_.insert({ busses_.back().name, &busses_.back() });
}



const Stop* TransportCatalogue::FindStop(std::string_view query) const {
	if (!stop_name_to_stop_.count(query)) {
		return nullptr;
	}
	return stop_name_to_stop_.at(query);
}

const Bus* TransportCatalogue::FindBus(std::string_view query) const {
	if (!bus_name_to_bus_.count(query)) {
		return nullptr;
	}
	return bus_name_to_bus_.at(query);
}



void TransportCatalogue::SetDistanceBetweenStops(const Stop* stop1, const Stop* stop2, unsigned	int distance) {
	distance_between_stops_[{stop1, stop2}] = distance;
}




std::set<std::string_view> TransportCatalogue::GetBusesOnStop(std::string_view query) const {
	using namespace std::literals;

	std::set<std::string_view> found_stop_names;

	for (auto& [bus_name, bus] : bus_name_to_bus_) { //пока тупо циклом, впоследствии можно заменить на что посерьезнее
		if (std::find_if(bus->stops.begin(), //тут может рухнуть производительность
						 bus->stops.end(), 
						 [&query](const Stop* stop) { return stop->name == query; }) != bus->stops.end()) 
		{
			found_stop_names.insert(bus_name);
		}
	}

	return found_stop_names;
}

double TransportCatalogue::GetDistanceBetweenStops(std::pair<const Stop*, const Stop*> pair_of_stops) const {
	if (!distance_between_stops_.count(pair_of_stops)) { return NAN; }

	return distance_between_stops_.at(pair_of_stops);
}

const Stop* TransportCatalogue::GetStopByID(size_t vertex_id) const {
	return &(stops_.at(vertex_id));
}

const distances_umap& TransportCatalogue::GetDistancesUmap() const {
	return distance_between_stops_;
}

const std::unordered_map<std::string_view, const Stop*>& TransportCatalogue::GetStopsUMap() const {
	return stop_name_to_stop_;
}

const std::deque<Stop>& TransportCatalogue::GetStopsDeque() const {
	return stops_;
}

const std::map<std::string_view, const Bus*>& TransportCatalogue::GetBusesMap() const {
	return bus_name_to_bus_;
}


double TransportCatalogue::ComputeActualDistanceBetweenNearbyStops(
	std::vector<const Stop*>::const_iterator stop_it0, 
	std::vector<const Stop*>::const_iterator stop_it1) const
{
	double distance = GetDistanceBetweenStops({ *stop_it0, *stop_it1 });
	if (!std::isnan(distance)) { //если нашли точно соответствующую дистанцию
		return distance;
	}
	else { //если не нашли точно соответствующую, ищем обратную дистанцию
		distance = GetDistanceBetweenStops({ *stop_it1, *stop_it0 });
		if (!std::isnan(distance)) {
			return distance;
		}
		else { //если не нашли никакую, считаем географию
			return geo::ComputeDistance((**stop_it0).coordinates, (**stop_it1).coordinates);
		}
	}
}