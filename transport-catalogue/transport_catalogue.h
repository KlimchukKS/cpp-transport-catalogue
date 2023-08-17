#pragma once

#include "domain.h"
#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

//класс транспортного справочника

namespace transport_catalogue {

    class TransportCatalogue {
    private:
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
        std::unordered_map<std::string_view, std::unordered_set<Bus*>> stop_and_stopping_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, StopsDistanceHasher> stops_distance_;

    public:
        void AddStop(std::tuple<std::string , double, double>& stop);
        void AddStop(std::tuple<std::string , double, double>&& stop);

        Stop FindStop(std::string_view);

        void AddBus(std::string& bus, std::vector<std::string>& stops, bool is_roundtrip);

        void SetDistanceBetweenStops(std::tuple<std::string, int, std::string>& stop_distance_to_stop);

        Bus FindBus(std::string_view);

        std::optional<BusInfo> GetBusInfo(std::string_view bus) const;

        const std::unordered_set<Bus*>* GetStopInfo(std::string_view stop) const;

        std::vector<const Bus*> GetBuses() const;

        std::vector<const Stop*> GetStopsIncludedInRoutes() const;

        size_t GetStopsCount() const {
            return stop_and_stopping_buses_.size();
        };

        std::optional<double> GetDistanceBetweenTwoStops(std::pair<std::string_view, std::string_view> stops) const {
            auto& [lhs, rhs] = stops;
            if (stops_distance_.count({stopname_to_stop_.at(lhs), stopname_to_stop_.at(rhs)})) {
                return stops_distance_.at({stopname_to_stop_.at(lhs), stopname_to_stop_.at(rhs)});
            }

            return std::nullopt;
        }
    };
}
