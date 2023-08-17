#pragma once

#include "geo.h"

#include <string_view>
#include <string>
#include <vector>
#include <unordered_set>

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


