#define _USE_MATH_DEFINES
#include <cmath>

#include "svg.h"

namespace svg {

    using namespace std::literals;

    //для StrokeLineCap
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
        using namespace std::literals;
        switch (cap) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
        }
        return out;
    }

    //для StrokeLineJoin 
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
        using namespace std::literals;
        switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl
            << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;

        RenderContext context(out, 2);
        for (const auto& object : objects_) {
            object->Render(context.Indented());
        }

        out << R"(</svg>)";
    }

    // ---------- Circle ------------------
    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << R"(<polyline points=")";

        bool is_first = true;
        for (const Point& point : points_) {
            if (!is_first) {
                out << ' ';
            }
            out << point.x << ',' << point.y;
            is_first = false;
        }
        out << '"';
        RenderAttrs(out);
        out << R"(/>)";
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;

        if (!data_.empty()) { //экранирование символов для XML
            for (size_t index = 0; index < data_.size(); ++index) {
                if (data_[index] == '"') {
                    data_.erase(index);
                    data.insert(index, "&quot;"s);
                }
                else if (data_[index] == '\'') {
                    data_.erase(index);
                    data.insert(index, "&apos;"s);
                }
                else if (data_[index] == '<') {
                    data_.erase(index);
                    data.insert(index, "&lt;"s);
                }
                else if (data_[index] == '>') {
                    data_.erase(index);
                    data.insert(index, "&gt;"s);
                }
                else if (data_[index] == '&') {
                    data_.erase(index);
                    data.insert(index, "&amp;"s);
                }
            }
        }

        return *this;
    }
    
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << R"(<text)";
        RenderAttrs(out);
        out << R"( x=")" << pos_.x << R"(" y=")" << pos_.y << R"(")";
        out << R"( dx=")" << offset_.x << R"(" dy=")" << offset_.y << R"(")";
        out << R"( font-size=")" << font_size_ << R"(")";
        if (!font_family_.empty()) {
            out << R"( font-family=")" << font_family_ << R"(")";
        }
        if (!font_weight_.empty()) {
            out << R"( font-weight=")" << font_weight_ << R"(")";
        }
        out << R"(>)";

        out << data_;

        out << R"(</text>)";
    }

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }

}  // namespace svg