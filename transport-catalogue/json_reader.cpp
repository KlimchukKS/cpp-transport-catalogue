#include "json_reader.h"

#include "domain.h"
#include "request_handler.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <algorithm>
#include <sstream>

using namespace transport_catalogue;
using namespace std;

void MapStatRequest::Print(json::Builder& builder) const {
    RequestHandler rh(db_, renderer_);
    std::stringstream strm;
    rh.RenderMap(strm);

    builder.StartDict()
          .Key("map"s).Value(strm.str())
          .Key("request_id"s).Value(id_).EndDict();
}

json::Array StopStatRequest::GetBusNames(const unordered_set<Bus*>* stop_buses) const {
    std::vector<std::string> tmp(stop_buses->size());
    std::transform(stop_buses->begin(), stop_buses->end(), tmp.begin(), [](Bus* bus){
        return bus->bus_name;
    });
    std::sort(tmp.begin(), tmp.end());

    json::Array buses;

    for (auto& bus : tmp) {
        buses.emplace_back(bus);
    }

    return buses;
}

void StopStatRequest::Print(json::Builder& builder) const {
    auto stop_buses = db_.GetStopInfo(stop_name_);
    if (stop_buses) {
        builder.StartDict()
                .Key("buses"s).Value(GetBusNames(stop_buses))
                .Key("request_id"s).Value(id_)
                .EndDict();
    } else {
        builder.StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id_)
                .EndDict();
    }
}

void BusStatRequest::Print(json::Builder& builder) const {
    auto bus_info = db_.GetBusInfo(route_name_);
    if (bus_info) {
        builder.StartDict()
                .Key("request_id"s).Value(id_)
                .Key("curvature"s).Value(bus_info->curvature)
                .Key("route_length"s).Value(bus_info->route_length)
                .Key("stop_count"s).Value(int(bus_info->stop_count))
                .Key("unique_stop_count"s).Value(int(bus_info->unique_stop_count))
                .EndDict();
    } else {
        builder.StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id_)
                .EndDict();
    }
}

void RoutingStatRequest::Print(json::Builder& builder) const {
    if (!route_) {
        builder.StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id_)
                .EndDict();
        return;
    }

    builder.StartDict().Key("items"s).StartArray();

    for (const auto& item : route_->items) {
        switch (item.item_type) {
            case Item::Type::kWait :
                builder.StartDict()
                        .Key("type"s).Value("Wait"s)
                        .Key("time"s).Value(item.time)
                        .Key("stop_name"s).Value(std::string(item.name))
                        .EndDict();
                break;
            case Item::Type::kBus:
                builder.StartDict()
                        .Key("type"s).Value("Bus"s)
                        .Key("span_count"s).Value(item.span_count)
                        .Key("time"s).Value(item.time)
                        .Key("bus"s).Value(std::string(item.name))
                        .EndDict();
        }
    }

    builder.EndArray()
            .Key("request_id"s).Value(id_)
            .Key("total_time"s).Value(route_->total_time)
            .EndDict();
}

JsonReader::JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& r)
        : db_(db), renderer_(r) {}

void JsonReader::ParseBaseRequests(const json::Node& input_node) {
    vector<tuple<string, int, string>> stop_distance_to_stop;
    std::vector<std::tuple<std::string, std::vector<std::string>, bool>> buses_and_stops;

    for (const auto& node : input_node.AsDict().at("base_requests").AsArray()) {
        auto map_stops_and_buses = node.AsDict();
        if (map_stops_and_buses.at("type"s) == "Stop") {
            string name_stop = map_stops_and_buses.at("name"s).AsString();
            db_.AddStop({name_stop,
                         map_stops_and_buses.at("latitude"s).AsDouble(),
                         map_stops_and_buses.at("longitude"s).AsDouble()});
            for (const auto& [to_stop, distances] : map_stops_and_buses.at("road_distances").AsDict()) {
                stop_distance_to_stop.push_back({name_stop, distances.AsInt(), to_stop});
            }
        } else {
            vector<string> stops;
            for (auto& node_str : map_stops_and_buses.at("stops"s).AsArray()) {
                stops.push_back(node_str.AsString());
            }

            if (!map_stops_and_buses.at("is_roundtrip"s).AsBool()) {
                std::vector<std::string> tmp = {stops.begin(), stops.end() - 1};
                tmp.insert(tmp.end(), stops.rbegin(), stops.rend());
                buses_and_stops.push_back({map_stops_and_buses.at("name"s).AsString(), move(tmp),
                                           map_stops_and_buses.at("is_roundtrip"s).AsBool()});
            } else {
                buses_and_stops.push_back({map_stops_and_buses.at("name"s).AsString(), move(stops),
                                           map_stops_and_buses.at("is_roundtrip"s).AsBool()});
            }
        }
    }

    for (auto& sds : stop_distance_to_stop) {
        db_.SetDistanceBetweenStops(sds);
    }

    for (auto& [bus, stops, is_roundtrip] : buses_and_stops) {
        db_.AddBus(bus, stops, is_roundtrip);
    }
}

void JsonReader::ParseStatRequests(const json::Node& input_node) {
    const RouteBuilder& route_builder = ParseRoutingSettingsAndGetRouteBuilder(input_node);

    for (auto& map_requests : input_node.AsDict().at("stat_requests").AsArray()) {
        if (map_requests.AsDict().at("type"s) == "Stop"s) {
            requests_data_.push_back(std::make_unique<StopStatRequest>(map_requests.AsDict().at("id"s).AsInt(),
                                                                       map_requests.AsDict().at("name"s).AsString(),
                                                                       db_));
        } else if (map_requests.AsDict().at("type"s) == "Bus"s) {
            requests_data_.push_back(std::make_unique<BusStatRequest>(map_requests.AsDict().at("id"s).AsInt(),
                                                                       map_requests.AsDict().at("name"s).AsString(),
                                                                       db_));
        } else if (map_requests.AsDict().at("type"s) == "Map"s) {
            requests_data_.push_back(std::make_unique<MapStatRequest>(map_requests.AsDict().at("id"s).AsInt(),
                                                                      db_,
                                                                      renderer_));
        } else if (map_requests.AsDict().at("type"s) == "Route"s) {
            requests_data_.push_back(std::make_unique<RoutingStatRequest>(map_requests.AsDict().at("id"s).AsInt(),
                    std::move(route_builder.GetRout(map_requests.AsDict().at("from"s).AsString(),
                                                    map_requests.AsDict().at("to"s).AsString()))));
        }
    }
}

void JsonReader::OutStatRequests(std::ostream& out) {
    auto stat_requests = json::Builder{};

    stat_requests.StartArray();

    for (auto& request : requests_data_) {
        request->Print(stat_requests);
    }

    json::Print(json::Document{stat_requests.EndArray().Build()}, out);
}

svg::Color JsonReader::GetColor(const json::Node& node) {
    if (node.IsArray()) {
        // Три - это количество элементов, необходимых для инициализации цвета в формате RGB
        if (node.AsArray().size() == 3) {
            return svg::Rgb{uint8_t(node.AsArray()[0].AsInt()),
                            uint8_t(node.AsArray()[1].AsInt()),
                            uint8_t(node.AsArray()[2].AsInt())};
        } else {
            return svg::Rgba{uint8_t(node.AsArray()[0].AsInt()),
                            uint8_t(node.AsArray()[1].AsInt()),
                            uint8_t(node.AsArray()[2].AsInt()),
                             node.AsArray()[3].AsDouble()};
        }
    } else if (node.IsString()) {
        return node.AsString();
    }
    return monostate{};
}

renderer::VisualizationSettings JsonReader::ParseRenderSettings(const json::Node& input_node) {
    auto& render_settings = input_node.AsDict().at("render_settings"s).AsDict();

    renderer::VisualizationSettings vs;

    vs.width = render_settings.at("width"s).AsDouble();
    vs.height = render_settings.at("height"s).AsDouble();
    vs.padding = render_settings.at("padding"s).AsDouble();
    vs.line_width = render_settings.at("line_width"s).AsDouble();
    vs.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    vs.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    vs.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    vs.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();

    auto bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
    vs.bus_label_offset.x = bus_label_offset[0].AsDouble();
    vs.bus_label_offset.y = bus_label_offset[1].AsDouble();

    auto stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
    vs.stop_label_offset.x = stop_label_offset[0].AsDouble();
    vs.stop_label_offset.y = stop_label_offset[1].AsDouble();

    vs.underlayer_color = GetColor(render_settings.at("underlayer_color"s));

    for (const auto& node : render_settings.at("color_palette"s).AsArray()) {
        vs.color_palette.push_back(GetColor(node));
    }

    return vs;
}

RouteBuilder JsonReader::ParseRoutingSettingsAndGetRouteBuilder(const json::Node& input_node) {
    auto& render_settings = input_node.AsDict().at("routing_settings"s).AsDict();

    RouteBuilder route_builder(db_,
            render_settings.at("bus_velocity"s).AsDouble(),
            render_settings.at("bus_wait_time"s).AsDouble());

    return route_builder;
}

void JsonReader::ParseJSON(std::istream &input) {
    const json::Document input_document = json::Load(input);

    this->ParseBaseRequests(input_document.GetRoot());

    this->ParseStatRequests(input_document.GetRoot());

    renderer_.SetVisualizationSettings(std::move(ParseRenderSettings(input_document.GetRoot())));
}
