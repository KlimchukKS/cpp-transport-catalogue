#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "svg.h"
#include "map_renderer.h"

#include <variant>

// request_handler — обрабатывает запросы. Играет роль Фасада, который упрощает взаимодействие с транспортным каталогом;

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);
    using BusPtr = transport_catalogue::Bus*;

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport_catalogue::BusInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через остановку
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
    
    void RenderMap(std::ostream& out);

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

    std::vector<geo::Coordinates> GetGeoCoords() const;
};
