#pragma once

#include "geo.h"
#include "svg.h"

#include <string_view>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

// domain — классы основных сущностей, описывают автобусы и остановки;

namespace transport_catalogue {

    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> stops;
        std::unordered_set<std::string_view> unique_stops;
        bool is_roundtrip;

        int route_length = 0;
        double geographic_distance = 0.;
    };

    struct BusInfo {
        std::string_view bus_name;
        double curvature;
        double route_length;
        size_t stop_count;
        size_t unique_stop_count;
    };
}

namespace renderer {
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
}

namespace serialization_data {

    struct RoadDistances {
        uint32_t name_stop_to = 1;
        int32_t distances = 2;
    };

    struct Stop {
        uint32_t name;
        geo::Coordinates coordinates;
        std::vector<RoadDistances> road_distances;
    };

    struct Bus {
        uint32_t name;
        std::vector<uint32_t> stops;
        bool is_roundtrip;
    };

   struct RouteSettings {
       double bus_velocity;
       double bus_wait_time;
    };

    struct SerializationData {
        std::vector<std::pair<uint32_t, std::string>> name_repository;
        std::vector<Stop> stops;
        std::vector<Bus> buses;
        renderer::VisualizationSettings vs;
        RouteSettings route_settings;
    };

}

struct Item {
    std::string_view name;
    int span_count;
    double time;

    enum class Type {
        kBus,
        kWait,
    };

    Type item_type;
};

struct Route {
    double total_time;

    std::vector<Item> items;
};


