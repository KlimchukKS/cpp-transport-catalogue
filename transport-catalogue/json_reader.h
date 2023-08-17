#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json_builder.h"

#include <sstream>
#include "request_handler.h"

// json.h выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов

class StatRequestData {
protected:
    int id_;
public:
    virtual ~StatRequestData() = default;

    StatRequestData(int id)
    : id_(id) {
    }

    virtual void Print(json::Builder& builder) const = 0;
};

class BusStatRequest : public StatRequestData {
    std::string route_name_;
    const transport_catalogue::TransportCatalogue& db_;
public:
    BusStatRequest(int id, const std::string& name, const transport_catalogue::TransportCatalogue& db)
            : StatRequestData(id), route_name_(name), db_(db) {
    }
    BusStatRequest(int id, std::string&& name, const transport_catalogue::TransportCatalogue& db)
            : StatRequestData(id), route_name_(std::move(name)), db_(db) {
    }

    void Print(json::Builder& builder) const override;
};

class StopStatRequest : public StatRequestData {
    using StatRequestData = StatRequestData;
    std::string stop_name_;
    const transport_catalogue::TransportCatalogue& db_;

    json::Array GetBusNames(const std::unordered_set<transport_catalogue::Bus*>* stop_buses) const;

public:
    StopStatRequest(int id, const std::string& name, const transport_catalogue::TransportCatalogue& db)
    : StatRequestData(id), stop_name_(name), db_(db) {
    }
    StopStatRequest(int id, std::string&& name, const transport_catalogue::TransportCatalogue& db)
    : StatRequestData(id), stop_name_(std::move(name)), db_(db) {
    }

    void Print(json::Builder& builder) const override;
};

class MapStatRequest : public StatRequestData {
    const transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

public:
    MapStatRequest(int id, transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
    : StatRequestData(id), db_(db), renderer_(renderer) {
    }

    void Print(json::Builder& builder) const override;
};

class RoutingStatRequest : public StatRequestData {
    std::optional<Route> route_;

public:
    RoutingStatRequest(int id, std::optional<Route> route)
    : StatRequestData(id), route_(std::move(route)) {
    }

    void Print(json::Builder& builder) const override;
};

class JsonReader {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& r);

    void ParseJSON(std::istream &input);

    void OutStatRequests(std::ostream& out);

private:
    transport_catalogue::TransportCatalogue& db_;

    renderer::MapRenderer& renderer_;

    std::vector<std::unique_ptr<StatRequestData>> requests_data_;

    void ParseBaseRequests(const json::Node& input_node);

    void ParseStatRequests(const json::Node& input_node);

    renderer::VisualizationSettings ParseRenderSettings(const json::Node& input_node);

    svg::Color GetColor(const json::Node& node);

    RouteBuilder ParseRoutingSettingsAndGetRouteBuilder(const json::Node& input_node);
};
