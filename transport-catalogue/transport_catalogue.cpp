#include "transport_catalogue.h"

#include <numeric>
#include <stdexcept>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::tuple<std::string , double, double>& stop) {
        auto& [name_stop, x, y] = stop;

        stops_.push_front({move(name_stop), {x, y}});

        stopname_to_stop_[stops_.front().stop_name] = &stops_.front();

        stop_and_stopping_buses_[stops_.front().stop_name];
    }

    void TransportCatalogue::AddBus(std::string& bus, std::vector<std::string>& stops) {
        std::vector<Stop*> new_stops;
        std::unordered_set<std::string_view> unique_stops;

        for (auto& stop : stops) {
            new_stops.push_back(stopname_to_stop_[stop]);
            unique_stops.insert(stopname_to_stop_[stop]->stop_name);
        }

        buses_.push_front(std::move(Bus{bus, new_stops, unique_stops}));
        busname_to_bus_[buses_.front().bus_name] = &buses_.front();

        for (auto stop : buses_.front().stops) {
            stop_and_stopping_buses_[stop->stop_name].insert(buses_.front().bus_name);
        }

        for (int i = 0, j = 1; j < busname_to_bus_.at(bus)->stops.size(); ++i, ++j) {
            Stop* const lhs = busname_to_bus_.at(bus)->stops[i];
            Stop* const rhs = busname_to_bus_.at(bus)->stops[j];

            busname_to_bus_.at(bus)->geographic_distance += ComputeDistance(lhs->coordinates, rhs->coordinates);

            if (stops_distance_.count({lhs, rhs})) {
                busname_to_bus_.at(bus)->route_length += stops_distance_.at({lhs, rhs});
            } else if (stops_distance_.count({rhs, lhs})) {
                busname_to_bus_.at(bus)->route_length += stops_distance_.at({rhs, lhs});
            }
        }
    }

    void TransportCatalogue::SetDistanceBetweenStops(std::tuple<std::string, int, std::string>& stop_distance_to_stop) {
        auto& [stop_first, distance, stop_second] = stop_distance_to_stop;
        stops_distance_[{stopname_to_stop_.at(stop_first), stopname_to_stop_.at(stop_second)}] = distance;
    }

    TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {
        if (!busname_to_bus_.count(bus)) {
            throw std::logic_error("Bus not found");
        }

        return {busname_to_bus_.at(bus)->bus_name,
                busname_to_bus_.at(bus)->stops.size(),
                busname_to_bus_.at(bus)->unique_stops.size(),
                busname_to_bus_.at(bus)->route_length,
                busname_to_bus_.at(bus)->route_length / busname_to_bus_.at(bus)->geographic_distance};
    }

    std::unordered_set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop) const {
        if (!stop_and_stopping_buses_.count(stop)) {
            throw std::logic_error("Stop not found");
        }

        return stop_and_stopping_buses_.at(stop);
    }
}
