#include "map_renderer.h"


bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void MapRenderer::DrawMap(std::ostream& out) {
    std::map<std::string_view, svg::Point> stop_name_to_point = GetStopnamesPointsToDraw();//координаты на экране - тут и исключение повторов, и сортировка по имени

    std::unordered_map < std::string_view, svg::Color> bus_names_to_color;

    //линия + заполнение мапы цветов маршрутов
    size_t color_counter = 0;
    for (const auto& [bus_name, bus_ptr] : catalogue_.GetBusesMap()) {
        if (bus_ptr->stops.empty()) {
            continue;
        }

        DrawBusLine(bus_ptr, color_counter, stop_name_to_point);

        bus_names_to_color.emplace(bus_name, render_settings_.color_palette.at(color_counter));

        ++color_counter;
        if (color_counter >= render_settings_.color_palette.size()) {
            color_counter = 0;
        }
    }

    //рисуем названия маршрутов
    for (const auto& [bus_name, point] : GetPointsForAllBusesNames(stop_name_to_point)) {
        DrawBusName(bus_name, point, bus_names_to_color.at(bus_name));
    }

    //круги остановок
    for (const auto& [stop_name, point] : stop_name_to_point) {
        DrawStopCircle(point);
    }

    //имена остановок
    for (const auto& [stop_name, point] : stop_name_to_point) {
        DrawStopName(stop_name, point);
    }

    all_objects_.Render(out);
}

void MapRenderer::DrawBusLine(const Bus* bus, size_t color_counter, const std::map<std::string_view, svg::Point>& stop_name_to_point) {

    //ломаная
    svg::Polyline poliline;
    for (const Stop* stop_ptr : bus->stops) {
        poliline.AddPoint(stop_name_to_point.at(stop_ptr->name));
    }
    //задать параметры линиии - через класс-родитель PathProps 
    poliline.SetStrokeColor(render_settings_.color_palette[color_counter]); //я люблю так прописывать! не хочу в одну строку!
    poliline.SetFillColor({});
    poliline.SetStrokeWidth(render_settings_.line_width);
    poliline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    poliline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    all_objects_.Add(std::move(poliline));
}

void MapRenderer::DrawBusName(const std::string_view& bus_name, const svg::Point& pos, const svg::Color& color) {
    svg::Text text_underlier;//подложка
    text_underlier.SetFillColor(render_settings_.underlayer_color);
    text_underlier.SetStrokeColor(render_settings_.underlayer_color);
    text_underlier.SetStrokeWidth(render_settings_.underlayer_width);
    text_underlier.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text_underlier.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text_underlier.SetOffset(render_settings_.bus_label_offset);
    text_underlier.SetFontSize(render_settings_.bus_label_font_size);
    text_underlier.SetFontFamily("Verdana");
    text_underlier.SetFontWeight("bold");

    svg::Text text;//осн текст
    text.SetFillColor(color);
    text.SetOffset(render_settings_.bus_label_offset);
    text.SetFontSize(render_settings_.bus_label_font_size);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");

    all_objects_.Add(std::move(text_underlier.SetPosition(pos).SetData(std::string(bus_name))));
    all_objects_.Add(std::move(text.SetPosition(pos).SetData(std::string(bus_name))));
}

void MapRenderer::DrawStopCircle(const svg::Point& pos) {
    //генерируем круги по координатам и суем их в all_objects
    svg::Circle stop_circle;
    stop_circle.SetRadius(render_settings_.stop_radius);
    stop_circle.SetFillColor("white");
    all_objects_.Add(std::move(stop_circle.SetCenter(pos)));
}

void MapRenderer::DrawStopName(const std::string_view& stop_name, const svg::Point& pos) {
    svg::Text text_underlier;//подложка
    text_underlier.SetFillColor(render_settings_.underlayer_color);
    text_underlier.SetStrokeColor(render_settings_.underlayer_color);
    text_underlier.SetStrokeWidth(render_settings_.underlayer_width);
    text_underlier.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text_underlier.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text_underlier.SetOffset(render_settings_.stop_label_offset);
    text_underlier.SetFontSize(render_settings_.stop_label_font_size);
    text_underlier.SetFontFamily("Verdana");

    svg::Text text;//осн текст
    text.SetFillColor("black");
    text.SetOffset(render_settings_.stop_label_offset);
    text.SetFontSize(render_settings_.stop_label_font_size);
    text.SetFontFamily("Verdana");

    text_underlier.SetPosition(pos);
    text_underlier.SetData(std::string(stop_name));
    all_objects_.Add(std::move(text_underlier));
    text.SetPosition(pos);
    text.SetData(std::string(stop_name));
    all_objects_.Add(std::move(text));
}

std::vector<std::pair<std::string_view, svg::Point>> MapRenderer::GetPointsForAllBusesNames(std::map<std::string_view, svg::Point>& stop_name_to_point) {
    std::vector<std::pair<std::string_view, svg::Point>> output;
    for (const auto& [bus_name, bus_ptr] : catalogue_.GetBusesMap()) {
        output.emplace_back(bus_ptr->name, stop_name_to_point.at(bus_ptr->stops.front()->name));
        if ((!bus_ptr->is_roundtrip) && (bus_ptr->stops.front() != bus_ptr->stops[(bus_ptr->stops.size() - 1) / 2])) {
            output.emplace_back(bus_ptr->name, stop_name_to_point.at(bus_ptr->stops[(bus_ptr->stops.size() - 1) / 2]->name));
        }
    }
    return output;
}

//получаем мапу названий остановок и их координат на экране + сортировка и исключение пустых остановок
std::map<std::string_view, svg::Point> MapRenderer::GetStopnamesPointsToDraw() {
    std::vector<geo::Coordinates> stops_coordinates;
    for (const auto& [stop_name, stop_ptr] : catalogue_.GetStopsUMap()) {
        if (!catalogue_.GetBusesOnStop(stop_name).empty()) {
            stops_coordinates.push_back(stop_ptr->coordinates);
        }
    }
    SphereProjector sphere_projector(stops_coordinates.begin(), stops_coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

    std::map<std::string_view, svg::Point> stop_name_to_point;//координаты на экране - тут и искалючение повторов, и сортировка по имени
    for (const auto& [stop_name, stop_ptr] : catalogue_.GetStopsUMap()) {
        if (!catalogue_.GetBusesOnStop(stop_name).empty()) {
            stop_name_to_point.emplace(stop_name, sphere_projector(stop_ptr->coordinates));
        }
    }
    return stop_name_to_point;
}
