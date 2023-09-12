#pragma once

#include <string>
#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <cmath>
#include <set>
#include <map>
#include <optional>

#include "domain.h"

struct PairDistanceHash {//хэшер для мапы с дистанциями
	size_t operator() (const std::pair<const Stop*, const Stop*>& pair_of_stop_ptrs) const {
		const int SIMPLE_MULTIPLIER = 13;
		size_t output = 0;

		unsigned int text_number0 = 0;
		for (const char& ch : pair_of_stop_ptrs.first->name) {
			int miltiplier = 1;
			text_number0 += (ch - 'A') * miltiplier;
			miltiplier *= 10;
		}
		unsigned int text_number1 = 0;
		for (const char& ch : pair_of_stop_ptrs.second->name) {
			int miltiplier = 1;
			text_number1 += (ch - 'A') * miltiplier;
			miltiplier *= 10;
		}

		output = text_number0 +	//names
			text_number1 * SIMPLE_MULTIPLIER +

			pair_of_stop_ptrs.first->coordinates.lat * pow(SIMPLE_MULTIPLIER, 2) +	//coordinates first
			pair_of_stop_ptrs.first->coordinates.lng * pow(SIMPLE_MULTIPLIER, 3) +

			pair_of_stop_ptrs.second->coordinates.lat * pow(SIMPLE_MULTIPLIER, 4) + //coordinates second
			pair_of_stop_ptrs.second->coordinates.lng * pow(SIMPLE_MULTIPLIER, 5);

		return hasher(output);
	}
	std::hash<size_t> hasher;
};

class TransportCatalogue {
public:
	using distances_umap = std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, PairDistanceHash>;


	//add
	void AddStop(Stop&& stop);	//добавляем остановку
	void AddBus(Bus&& bus);	//добавляем маршрут

	//find
	const Stop* FindStop(std::string_view query) const;	//поиск остановки по названию
	const Bus* FindBus(std::string_view query) const;	//поиск маршрута по названию

	//set
	void SetDistanceBetweenStops(const Stop* stop1, const Stop* stop2, unsigned	int distance);

	//get
	std::set<std::string_view> GetBusesOnStop(std::string_view query) const;
	double GetDistanceBetweenStops(std::pair<const Stop*, const Stop*> pair_of_stops) const; //получение фактического расстояния между двумя остановками. Если нет - возвращает NAN
	const distances_umap& GetDistancesUmap() const;
	const std::unordered_map<std::string_view, const Stop*>& GetStopsUMap() const;
	const std::deque<Stop>& GetStopsDeque() const;
	const std::map<std::string_view, const Bus*>& GetBusesMap() const;
	const Stop* GetStopByID(size_t vertex_id) const;

	//other
		//возвращает заданную вручную дистанцию, а если таковой нет - географическую
	double ComputeActualDistanceBetweenNearbyStops(
		std::vector<const Stop*>::const_iterator stop_it0, 
		std::vector<const Stop*>::const_iterator stop_it1
	) const;

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_; //для быстрого поиска остановок по названию

	std::deque<Bus> busses_;
	std::map<std::string_view, const Bus*> bus_name_to_bus_; //для быстрого поиска маршрутов по названию
	//просто мапа, т.к. нужна сортировка по названию при отрисовке svg

	std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, PairDistanceHash> distance_between_stops_; //ручное задание расстояния между остановками
};