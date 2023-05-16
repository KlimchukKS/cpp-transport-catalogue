#include "map_renderer.h"

using namespace std;

using namespace svg;
using namespace transport_catalogue;

namespace renderer {

    Route::Route(std::vector<svg::Point> p, double width, svg::Color route_color)
    : points_(std::move(p)), width_(width), route_color_(std::move(route_color)) {
    }

    RouteName::RouteName(RouteNameData data, const VisualizationSettings& vs)
    : data_(data),  vs_(vs){
    }

    StopSymbol::StopSymbol(svg::Point coord, double stop_radius)
    : coord_(coord), stop_radius_(stop_radius) {
    }

    StopName::StopName(std::string stop_name, svg::Point position, const VisualizationSettings& vs)
    : stop_name_(stop_name), position_(position), vs_(vs) {
    }

    void MapRenderer::SetVisualizationSettings(VisualizationSettings&& vs) {
        vs_ = std::move(vs);
    }

    void MapRenderer::RenderRouteLinesAndName(const SphereProjector& proj, const std::vector<const Bus*>& buses) {
        vector<unique_ptr<svg::Drawable>> route_lines;
        vector<unique_ptr<svg::Drawable>> route_name_text;

        auto it_color_palette = vs_.color_palette.begin();

        for (const auto bus : buses) {
            if (bus->stops.empty()) {
                continue;
            }

            std::vector<svg::Point> coords;
            for (auto stop : bus->stops) {
                coords.push_back(proj(stop->coordinates));
            }
            route_lines.emplace_back(make_unique<Route>(std::move(coords), vs_.line_width, *it_color_palette));

            RouteNameData route_name_data;
            route_name_data.route_name = bus->bus_name;
            route_name_data.route_color = *it_color_palette;
            route_name_data.position = proj(bus->stops.front()->coordinates);
            route_name_text.emplace_back(make_unique<RouteName>(route_name_data, vs_));

            size_t terminus = bus->stops.size() / 2;
            if (!bus->is_roundtrip && bus->stops[terminus] != bus->stops.front()) {
                route_name_data.position = proj(bus->stops[terminus]->coordinates);
                route_name_text.emplace_back(make_unique<RouteName>(route_name_data, vs_));
            }

            if (++it_color_palette == vs_.color_palette.end()) {
                it_color_palette = vs_.color_palette.begin();
            }
        }

        DrawPicture(route_lines, doc_);
        DrawPicture(route_name_text, doc_);
    }

    void MapRenderer::Render(const std::vector<const Bus*>& buses,
                             const std::vector<const Stop*>& stops,
                             const std::vector<geo::Coordinates>& geo_coords,
                             std::ostream& out) {
        const SphereProjector proj{
                geo_coords.begin(), geo_coords.end(), vs_.width, vs_.height, vs_.padding
        };

        RenderRouteLinesAndName(proj, buses);

        vector<unique_ptr<svg::Drawable>> stops_symbols;
        vector<unique_ptr<svg::Drawable>> stops_names;

        for (const auto stop : stops) {
            stops_symbols.emplace_back(make_unique<StopSymbol>(proj(stop->coordinates), vs_.stop_radius));
            stops_names.emplace_back(make_unique<StopName>(stop->stop_name, proj(stop->coordinates), vs_));
        }

        DrawPicture(stops_symbols, doc_);
        DrawPicture(stops_names, doc_);

        doc_.Render(out);
    }
}
