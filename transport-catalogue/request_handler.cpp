#include "request_handler.h"

#include <stdexcept>

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {}

std::optional<transport_catalogue::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetBusInfo(bus_name);
}

const std::unordered_set<RequestHandler::BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetStopInfo(stop_name);
}

void RequestHandler::RenderMap(std::ostream& out) {

    auto stops = db_.GetStopsIncludedInRoutes();

    std::sort(stops.begin(), stops.end(), [] (const transport_catalogue::Stop* lhs,
                                                             const transport_catalogue::Stop* rhs) {
        return lhs->stop_name < rhs->stop_name;
    });

    return renderer_.Render(db_.GetBuses(),
                            stops,
                            this->GetGeoCoords(),
                            out);
}

std::vector<geo::Coordinates> RequestHandler::GetGeoCoords() const {
    auto stops = db_.GetStopsIncludedInRoutes();

    std::vector<geo::Coordinates> coords(stops.size());

    std::transform(stops.begin(), stops.end(), coords.begin(), [](const transport_catalogue::Stop* stop){
        return stop->coordinates;
    });

    return coords;
}
