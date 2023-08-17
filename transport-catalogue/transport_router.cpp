#include "transport_router.h"

RouteBuilder::RouteBuilder(const transport_catalogue::TransportCatalogue& db, double bus_velocity , double bus_wait_time)
        : db_(db), bus_velocity_(bus_velocity), bus_wait_time_(bus_wait_time) {
    const auto& stops = db_.GetStopsIncludedInRoutes();

    id_bus_stop_entrance_.reserve(db_.GetStopsCount());

    graph_ = std::unique_ptr<graph::DirectedWeightedGraph<double>>(new graph::DirectedWeightedGraph<double>{db_.GetStopsIncludedInRoutes().size() * 2});

    {
        size_t counter = 0;

        for (auto stop : stops) {
            id_bus_stop_entrance_[stop->stop_name] = counter;
            graph_->AddEdge({counter,
                            counter + 1,
                            bus_wait_time_,
                            stop->stop_name,
                            0});
            counter += 2;
        }
    }

    const auto& buses = db_.GetBuses();

    for (auto bus : buses) {
        if (!bus->is_roundtrip) {
            BuildGraphEdgesIsNotRoundtrip(bus->stops, bus->bus_name, 0, (bus->stops.size() / 2) + 1);
            BuildGraphEdgesIsNotRoundtrip(bus->stops, bus->bus_name, bus->stops.size() / 2, bus->stops.size());
        } else {
            BuildGraphEdgesIsRoundtrip(bus->stops, bus->bus_name);
        }
    }

    router_ = std::unique_ptr<graph::Router<double>>(new graph::Router<double>{*graph_.get()});
}

double  RouteBuilder::CalculateEdgeTravelTime(double weight) const {
    const static double minutes_in_one_hour = 60.;
    const static double meters_in_one_kilometer = 1000.;

    return (weight * minutes_in_one_hour) / (meters_in_one_kilometer * bus_velocity_);
}

void RouteBuilder::BuildEdge(const std::vector<transport_catalogue::Stop*>& route_stops, std::string_view bus_name, size_t from, size_t to) {
    double weight = 0.;

    size_t span_count = 0;

    for (int lhs = from, rhs = from + 1; route_stops[lhs] != route_stops[to]; ++lhs, ++rhs) {
        if (rhs == route_stops.size()) {
            lhs = 0; rhs = 1;
        }
        auto distance = db_.GetDistanceBetweenTwoStops({route_stops[lhs]->stop_name,
                                                        route_stops[rhs]->stop_name});
        if (!distance) {
            distance = db_.GetDistanceBetweenTwoStops({route_stops[rhs]->stop_name,
                                                       route_stops[lhs]->stop_name});
        }

        weight += distance.value();
        ++span_count;
    }

    weight = CalculateEdgeTravelTime(weight);

    graph_->AddEdge({id_bus_stop_entrance_[route_stops[from]->stop_name] + 1,
                    id_bus_stop_entrance_[route_stops[to]->stop_name],
                    weight,
                    bus_name,
                    span_count});
}

void RouteBuilder::BuildGraphEdgesIsNotRoundtrip(const std::vector<transport_catalogue::Stop*>& route_stops, std::string_view bus_name, size_t begin, size_t end) {
    for (size_t i = begin; i < end; ++i) {
        for (size_t j = i; j < end; ++j) {
            if (i == j) {
                continue;
            }

            BuildEdge(route_stops, bus_name, i, j);
        }
    }
}

void RouteBuilder::BuildGraphEdgesIsRoundtrip(const std::vector<transport_catalogue::Stop*>& route_stops, std::string_view bus_name) {

    for (size_t i = 0; i < route_stops.size() - 1; ++i) {
        for (size_t j = i + 1; j < route_stops.size(); ++j) {
            if (i == j || (i == 0 && j == route_stops.size() - 1)) {
                continue;
            }

            BuildEdge(route_stops, bus_name, i, j);
        }
    }

}

std::optional<Route> RouteBuilder::GetRout(std::string_view from, std::string_view to) const {
    if (!id_bus_stop_entrance_.count(from) || !id_bus_stop_entrance_.count(to)) {
        return std::nullopt;
    }
    if (from == to) {
        return Route{0, {}};
    }

    std::optional<graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(id_bus_stop_entrance_.at(from),
                                                                                     id_bus_stop_entrance_.at(to) + 1);

    if (!route_info) {
        return std::nullopt;
    }

    Route route;

    route.total_time = route_info->weight - bus_wait_time_;

    route.items.reserve(route_info.value().edges.size());

    for (auto& id_edge : route_info.value().edges) {
        const auto& edge = graph_->GetEdge(id_edge);

        Item item;
        item.name = edge.name;
        item.span_count = edge.span_count;
        item.time = edge.weight;

        item.item_type = (id_bus_stop_entrance_.count(edge.name)) ? Item::Type::kWait : Item::Type::kBus;

        route.items.push_back(std::move(item));
    }
    route.items.resize(route.items.size() - 1);

    return route;
}
