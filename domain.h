#pragma once

#include "geo.h"
#include "svg.h"

#include <string>
#include <string_view>
#include <vector>


struct Stop {
	std::string name;
	geo::Coordinates coordinates;
	long unsigned int VertexId = 0; //уникальный ID остановки (по сути порядковый номер), по которому ориентируется роутер

	bool operator==(const Stop& other) const {
		return coordinates == other.coordinates && name == other.name;
	}

	bool operator!=(const Stop& other) const {
		return !(*this == other);
	}
};

//имя, вектор остановок, кол-во уникальных остановок на маршруте, длина маршрута фактическая, извилистость
struct Bus {
	std::string name;
	std::vector<const Stop*> stops;	//полная последовательность остановок
	unsigned int unique_stops_number = 0; //кол-во уникальных остановок на маршруте
	double route_length = 0; //длина маршрута фактическая (с учетом вручную заданного расстояния между остановками) - вычисляем в момент добавления маршрута
	double curvature = 1; //извилистость - отношение фактической длины маршрута к географическому расстоянию
	bool is_roundtrip = false; //если ложь - маршрут "зеркальный", если истина - кольцо
};

struct RenderSettings {
	double width = 0.0; //ширина и высота изображения в пикселях
	double height = 0.0;

	double padding = 0.0; //отступ краёв карты от границ SVG-документа

	double line_width = 0.0; //толщина линий, которыми рисуются автобусные маршруты
	double stop_radius = 0.0; //радиус окружностей, которыми обозначаются остановки

	int bus_label_font_size = 0; //размер текста, которым написаны названия автобусных маршрутов
	svg::Point bus_label_offset; //смещение надписи с названием маршрута относительно координат конечной остановки на карте

	int stop_label_font_size = 0; //размер текста, которым отображаются названия остановок
	svg::Point stop_label_offset; //смещение названия остановки относительно её координат на карте

	svg::Color underlayer_color; //цвет подложки под названиями остановок и маршрутов
	double underlayer_width = 0.0; //толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>

	std::vector<svg::Color> color_palette; //цветовая палитра для выбора цветов маршрутов
};

struct RoutingSettings {
	double bus_wait_time = 0; //минуты
	double bus_velocity = 0; //километры в час
};

struct SerializationSettings {
	std::string file_name;
};