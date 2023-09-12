#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <optional>
#include <variant>


namespace svg {
    struct Rgb {
        Rgb() = default;

        Rgb(uint8_t red, uint8_t green, uint8_t blue)
            : red(red)
            , green(green)
            , blue(blue)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;

        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
            : red(red)
            , green(green)
            , blue(blue)
            , opacity(opacity)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };
    
    //variant<std::monostate, std::string, svg::Rgb, svg::Rgba>
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>; //monostate = "none"s

    inline const Color NoneColor{ "none" };

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none";
        }

        void operator()(std::string s) const {
            out << s;
        }

        void operator()(svg::Rgb color) const {
            out << "rgb(" << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ')';
        }

        void operator()(svg::Rgba color) const {
            out << "rgba(" << static_cast<unsigned>(color.red) << ',' << static_cast<unsigned>(color.green) << ',' << static_cast<unsigned>(color.blue) << ',' << color.opacity << ')';
        }
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    //для StrokeLineCap
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap);

    //для StrokeLineJoin 
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join);
    
    struct Point { //double x, y
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }

        bool operator==(const Point& other) {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Point& other) {
            return !(*this == other);
        }

        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Object_template>
        void Add(Object_template object);

        virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& container) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        PathProps() = default;

        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(ColorPrinter{out}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(ColorPrinter{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle() = default;

        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline() = default;

        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text() = default;

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_;
        Point offset_;
        std::string font_family_;
        uint32_t font_size_ = 1;
        std::string font_weight_;
        std::string data_;

    };

    class Document : public ObjectContainer {
    public:
        Document() = default;
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_ = {}; //УКАЗАТЕЛИ НА ОБЪЕКТЫ
    };

    template <typename Object_template>
    void ObjectContainer::Add(Object_template object) {
        AddPtr(std::make_unique<Object_template>(std::move(object)));
    }

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);
}  // namespace svg


namespace shapes {

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1)
            , p2_(p2)
            , p3_(p3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
        }

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
            using namespace std::literals;

            polyline_ = CreateStar(center, outer_rad, inner_rad, num_rays);
            polyline_.SetFillColor("red"s);
            polyline_.SetStrokeColor("black"s);
        }

        virtual ~Star() = default;

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(polyline_);
        }
    private:
        svg::Polyline polyline_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point head_center, double head_radius) {
            using namespace std::literals;

            head_.SetCenter(head_center);
            head_.SetRadius(head_radius);
            head_.SetFillColor(svg::Rgb(240,240,240));
            head_.SetStrokeColor("black"s);

            middle_.SetCenter({ head_center.x, head_center.y + 2.0 * head_radius });
            middle_.SetRadius(head_radius * 1.5);
            middle_.SetFillColor(svg::Rgb(240, 240, 240));
            middle_.SetStrokeColor("black"s);

            bottom_.SetCenter({ head_center.x, head_center.y + 5.0 * head_radius });
            bottom_.SetRadius(head_radius * 2.0);
            bottom_.SetFillColor(svg::Rgb(240, 240, 240));
            bottom_.SetStrokeColor("black"s);
        }

        virtual ~Snowman() = default;

        void Draw(svg::ObjectContainer& container) const override { //рисуем с нижнего круга
            container.Add(bottom_);
            container.Add(middle_);
            container.Add(head_);
        }

    private:
        svg::Circle head_;
        svg::Circle middle_;
        svg::Circle bottom_;
    };

} // namespace shapes 
