#pragma once

#include "svg.h"
#include "domain.h"
#include "geo.h"

#include <memory>
#include <vector>
#include <utility>
#include <algorithm>

namespace renderer {

    inline const double EPSILON = 1e-6;

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!(std::abs((max_lon - min_lon_)) < EPSILON)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!(std::abs((max_lat_ - min_lat)) < EPSILON)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct VisualizationSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        svg::Point bus_label_offset;
        int stop_label_font_size;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class Route : public svg::Drawable {
    public:
        Route(std::vector<svg::Point> p, double width, svg::Color route_color);

        void Draw(svg::ObjectContainer& container) const override {
            svg::Polyline polyline;
            for (const svg::Point& p : points_) {
                polyline.AddPoint(p);
            }
            container.Add(polyline.SetFillColor("none")
                                  .SetStrokeColor(route_color_)
                                  .SetStrokeWidth(width_)
                                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        }

    private:
        std::vector<svg::Point> points_;
        double width_;
        svg::Color route_color_;
    };

    struct RouteNameData {
        std::string route_name;
        svg::Point position;
        svg::Color route_color;
    };

    class RouteName : public svg::Drawable {
    public:
        RouteName(RouteNameData data, const VisualizationSettings& vs);

        void Draw(svg::ObjectContainer& container) const override {
            using namespace std::string_literals;

            const svg::Text base_text =  //
                    svg::Text()
                            .SetFontFamily("Verdana"s)
                            .SetFontSize(vs_.bus_label_font_size)
                            .SetPosition(data_.position)
                            .SetOffset(vs_.bus_label_offset)
                            .SetData(data_.route_name)
                            .SetFontWeight("bold"s);

            container.Add(svg::Text{base_text}
                            .SetStrokeColor(vs_.underlayer_color)
                            .SetFillColor(vs_.underlayer_color)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeWidth(vs_.underlayer_width));

            container.Add(svg::Text{base_text}.SetFillColor(data_.route_color));
        }

    private:
        RouteNameData data_;
        const VisualizationSettings& vs_;
    };

    class StopSymbol : public svg::Drawable {
    public:
        StopSymbol(svg::Point coord, double stop_radius);

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Circle()
                                  .SetCenter(coord_)
                                  .SetRadius(stop_radius_)
                                  .SetFillColor(fill_));
        }

    private:
        svg::Point coord_;
        double stop_radius_;
        const svg::Color fill_ = "white";
    };

    class StopName : public svg::Drawable {
    public:
        StopName(std::string stop_name, svg::Point position, const VisualizationSettings& vs);

        void Draw(svg::ObjectContainer& container) const override {
            using namespace std::string_literals;

            const svg::Text base_text =  //
                    svg::Text()
                            .SetFontFamily(font_family)
                            .SetFontSize(vs_.stop_label_font_size)
                            .SetPosition(position_)
                            .SetOffset(vs_.stop_label_offset)
                            .SetData(stop_name_);

            container.Add(svg::Text{base_text}
                                  .SetStrokeColor(vs_.underlayer_color)
                                  .SetFillColor(vs_.underlayer_color)
                                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                  .SetStrokeWidth(vs_.underlayer_width));

            container.Add(svg::Text{base_text}.SetFillColor(fill_));
        }

    private:
        std::string stop_name_;
        svg::Point position_;
        const VisualizationSettings& vs_;

        const std::string font_family = "Verdana";
        const svg::Color fill_ = "black";
    };

    class MapRenderer {
    public:
        void SetVisualizationSettings(VisualizationSettings&& vs);

        void Render(const std::vector<const transport_catalogue::Bus*>& buses,
                    const std::vector<const transport_catalogue::Stop*>& stops,
                    const std::vector<geo::Coordinates>& geo_coords,
                    std::ostream& out);

    private:
        VisualizationSettings vs_;
        svg::Document doc_;

        void RenderRouteLinesAndName(const SphereProjector& proj, const std::vector<const transport_catalogue::Bus*>& buses);

        template <typename DrawableIterator>
        void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) const {
            for (auto it = begin; it != end; ++it) {
                (*it)->Draw(target);
            }
        }

        template <typename Container>
        void DrawPicture(const Container& container, svg::ObjectContainer& target) const {
            using namespace std;
            DrawPicture(begin(container), end(container), target);
        }
    };
}
