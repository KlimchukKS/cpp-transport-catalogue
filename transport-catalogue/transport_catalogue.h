#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

//класс транспортного справочника

namespace transport_catalogue {

    class TransportCatalogue {
    private:
        using BusInfo = std::tuple<std::string_view, int, int, int, double>;

        struct Stop {
            std::string stop_name;
            Coordinates coordinates;
        };

        struct Bus {
            std::string bus_name;
            std::vector<Stop*> stops;
            std::unordered_set<std::string_view> unique_stops;

            int route_length = 0;
            double geographic_distance = 0.;
        };

        struct StopsDistanceHasher {
            size_t operator() (const std::pair<Stop*, Stop*> stops) const {
                size_t h_x = d_hasher_(stops.first);
                size_t h_y = d_hasher_(stops.second);

                return h_x + h_y * 1000000007;
            }

        private:
            std::hash<const void*> d_hasher_;
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_and_stopping_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, StopsDistanceHasher> stops_distance_;

    public:
        void AddStop(std::tuple<std::string , double, double>& stop);

        Stop FindStop(std::string_view);

        void AddBus(std::string& bus, std::vector<std::string>& stops);

        void SetDistanceBetweenStops(std::tuple<std::string, int, std::string>& stop_distance_to_stop);

        Bus FindBus(std::string_view);

        BusInfo GetBusInfo(std::string_view bus) const;

        std::unordered_set<std::string_view> GetStopInfo(std::string_view stop) const;
    };
}
