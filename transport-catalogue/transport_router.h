#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

#include <string_view>
#include <vector>
#include <memory>

class RouteBuilder {
private:
    const transport_catalogue::TransportCatalogue& db_;

    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    const double bus_velocity_;
    const double bus_wait_time_;

    std::unordered_map<std::string_view, size_t> id_bus_stop_entrance_;

    void BuildEdge(const std::vector<transport_catalogue::Stop*>& route_stops,
                   std::string_view bus_name,
                   size_t begin,
                   size_t end);

    void BuildGraphEdgesIsNotRoundtrip(const std::vector<transport_catalogue::Stop*>& route_stops,
                                       std::string_view bus_name,
                                       size_t begin,
                                       size_t end);

    void BuildGraphEdgesIsRoundtrip(const std::vector<transport_catalogue::Stop*>& route_stops,
                                    std::string_view bus_name);

    double CalculateEdgeTravelTime(double weight) const;

public:
    RouteBuilder(const transport_catalogue::TransportCatalogue& db, double bus_velocity , double bus_wait_time);

    std::optional<Route> GetRout(std::string_view from, std::string_view to) const;
};
