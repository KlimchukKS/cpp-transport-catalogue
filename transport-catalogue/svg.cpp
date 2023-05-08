#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream &out, const StrokeLineCap stroke_line_cap) {
        using namespace std::string_view_literals;

        switch (stroke_line_cap) {
            case StrokeLineCap::BUTT:
                return out << "butt"sv;
            case StrokeLineCap::ROUND:
                return out << "round"sv;
            case StrokeLineCap::SQUARE:
                return out << "square"sv;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream &out, const StrokeLineJoin stroke_line_join) {
        using namespace std::string_view_literals;

        switch (stroke_line_join) {
            case StrokeLineJoin::ARCS:
                return out << "arcs"sv;
            case StrokeLineJoin::BEVEL:
                return out << "bevel"sv;
            case StrokeLineJoin::MITER:
                return out << "miter"sv;
            case StrokeLineJoin::MITER_CLIP:
                return out << "miter-clip"sv;
            case StrokeLineJoin::ROUND:
                return out << "round"sv;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream &out, const Color color) {
        using namespace std::string_view_literals;
        if (const auto* value = std::get_if<std::string>(&color)) {
            out << *value;
        } else if (const auto* value = std::get_if<Rgb>(&color)) {
            out << "rgb("sv << uint16_t(value->red) << ","sv << uint16_t(value->green) << ","sv << uint16_t(value->blue) << ")"sv;
        } else if (const auto* value = std::get_if<Rgba>(&color)) {
            out << "rgba("sv << uint16_t(value->red) << ","sv << uint16_t(value->green) << ","sv << uint16_t(value->blue) << ","sv << value->opacity << ")"sv;
        } else {
            out << NoneColor;
        }
        return out;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

// ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            for (size_t i = 0; i < points_.size() - size_t(1); ++i) {
                out << points_[i].x << ","sv << points_[i].y << " "sv;
            }
            out << points_.back().x << ","sv << points_.back().y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
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
        size_ = size;
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
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
        out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
        out << "\" font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << data_ << "</text>"sv;
    }

// ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }


}  // namespace svg
