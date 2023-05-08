#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"

// json.h выполняет разбор JSON-данных, построенных в ходе парсинга, и формирует массив JSON-ответов

enum class RequestType {
    kBus,
    kStop,
    kMap
};

struct RequestsData {
    int id;
    RequestType type;
    std::string name;
};


class JsonReader {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& r);

    void ParseJSON(std::istream &input);

    void OutStatRequests(std::ostream& out);

private:
    transport_catalogue::TransportCatalogue& db_;

    renderer::MapRenderer& renderer_;

    std::vector<RequestsData> requests_data_;

    void ParseBaseRequests(const json::Node& input_node);

    void ParseStatRequests(const json::Node& input_node);

    json::Array GetBusNames(const std::unordered_set<transport_catalogue::Bus*>* stop_buses) const;

    renderer::VisualizationSettings ParseRenderSettings(const json::Node& input_node);

    svg::Color GetColor(const json::Node& node);
};
