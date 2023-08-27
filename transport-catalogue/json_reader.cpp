#include "json_reader.h"

#include "domain.h"
#include "request_handler.h"

#include "serialization.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <algorithm>

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

void JsonReader::ParseStatRequests(const json::Node& input_node, serialization_data::RouteSettings rs) {
    //const RouteBuilder& route_builder = ParseRoutingSettingsAndGetRouteBuilder(input_node);

    const RouteBuilder route_builder(db_, rs.bus_velocity, rs.bus_wait_time);

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
/*
void JsonReader::ParseJSON(std::istream &input) {
    const json::Document input_document = json::Load(input);

    this->ParseBaseRequests(input_document.GetRoot());

    this->ParseStatRequests(input_document.GetRoot());

    renderer_.SetVisualizationSettings(std::move(ParseRenderSettings(input_document.GetRoot())));
}
*/
std::pair<std::string, serialization_data::SerializationData> JsonReader::ParseJSONtoGetDataForSerialization(std::istream &input) {
    const json::Document input_document = json::Load(input);
    const auto& input_node = input_document.GetRoot();

    std::unordered_map<std::string_view, uint32_t> name_id;
    std::unordered_map<uint32_t, std::string> name_rep;

    serialization_data::SerializationData serialization_data;

    uint32_t counter = 0;

    for (const auto& node : input_node.AsDict().at("base_requests").AsArray()) {
        auto map_stops_and_buses = node.AsDict();
        if (map_stops_and_buses.at("type"s) == "Stop") {
            serialization_data::Stop stop;
            {// Дополнительное вложение нужно, чтобы убрать переменную name_stop. Так меньше шансов присвоить её, куда-то не туда.
                string_view name_stop = map_stops_and_buses.at("name"s).AsString();

                if (!name_id.count(name_stop)) {
                    name_rep[counter] = name_stop;
                    name_id[name_rep[counter]] = counter;
                    ++counter;
                }

                stop.name = name_id[name_stop];
            }
            stop.coordinates.lat = map_stops_and_buses.at("latitude"s).AsDouble();
            stop.coordinates.lng = map_stops_and_buses.at("longitude"s).AsDouble();

            for (const auto& [to_stop, distances] : map_stops_and_buses.at("road_distances").AsDict()) {
                if (!name_id.count(to_stop)) {
                    name_rep[counter] = to_stop;
                    name_id[name_rep[counter]] = counter;
                    ++counter;
                }

                stop.road_distances.push_back({name_id[to_stop],
                                               distances.AsInt()});
            }

            serialization_data.stops.push_back(std::move(stop));
        } else {
            serialization_data::Bus bus;
            {
                string_view name_bus = map_stops_and_buses.at("name"s).AsString();
                if (!name_id.count(name_bus)) {
                    name_rep[counter] = name_bus;
                    name_id[name_rep[counter]] = counter;
                    ++counter;
                }
                bus.name = name_id[name_bus];
            }

            std::vector<uint32_t> stops;
            for (auto& node_str : map_stops_and_buses.at("stops"s).AsArray()) {
                string_view name_stop = node_str.AsString();
                if (!name_id.count(name_stop)) {
                    name_rep[counter] = name_stop;
                    name_id[name_rep[counter]] = counter;
                    ++counter;
                }
                stops.push_back(name_id[name_stop]);
            }

            bus.is_roundtrip = map_stops_and_buses.at("is_roundtrip"s).AsBool();

            if (!bus.is_roundtrip) {
                std::vector<uint32_t> tmp = {stops.begin(), stops.end() - 1};
                tmp.insert(tmp.end(), stops.rbegin(), stops.rend());
                bus.stops = std::move(tmp);
            } else {
                bus.stops = std::move(stops);
            }
            serialization_data.buses.push_back(std::move(bus));
        }
    }

    std::vector<std::pair<uint32_t, std::string>> tmp{name_rep.begin(), name_rep.end()};
    serialization_data.name_repository = std::move(tmp);

    std::string serialization_setting = input_node.AsDict().at("serialization_settings").AsDict().at("file").AsString();

    serialization_data.vs = ParseRenderSettings(input_node);

    auto& render_settings = input_node.AsDict().at("routing_settings"s).AsDict();
    serialization_data.route_settings.bus_velocity = render_settings.at("bus_velocity"s).AsDouble();
    serialization_data.route_settings.bus_wait_time = render_settings.at("bus_wait_time"s).AsDouble();

    return {serialization_setting, serialization_data};
}

serialization_data::RouteSettings JsonReader::ParseDeserializeData(serialization_data::SerializationData&& data) {
    std::unordered_map<uint32_t, std::string> id_names(data.name_repository.begin(), data.name_repository.end());

    vector<tuple<string, int, string>> stop_distance_to_stop;
    std::vector<std::tuple<std::string, std::vector<std::string>, bool>> buses_and_stops;

    for (auto& [name, coord, r_d] : data.stops) {
        db_.AddStop({id_names[name],
                     coord.lat,
                     coord.lng});
        for (auto [to_stop, distances] : r_d) {
            stop_distance_to_stop.push_back({id_names[name], distances, id_names[to_stop]});
        }
    }

    for (auto& [name, stops_id, is_roundtrip] : data.buses) {
        vector<string> stops;
        for (auto& node_str : stops_id) {
            stops.push_back(id_names[node_str]);
        }

        buses_and_stops.push_back({id_names[name], move(stops), is_roundtrip});
    }

    for (auto& sds : stop_distance_to_stop) {
        db_.SetDistanceBetweenStops(sds);
    }

    for (auto& [bus, stops, is_roundtrip] : buses_and_stops) {
        db_.AddBus(bus, stops, is_roundtrip);
    }

    renderer_.SetVisualizationSettings(std::move(data.vs));

    serialization_data::RouteSettings rs = data.route_settings;

    return rs;
}

void JsonReader::ParseJsonProcessRequests(std::istream &input) {
    const json::Document input_document = json::Load(input);
    const auto& input_node = input_document.GetRoot();

    std::string serialization_setting = input_node.AsDict().at("serialization_settings").AsDict().at("file").AsString();

    auto rs = ParseDeserializeData(std::move(Deserialize(serialization_setting)));

    this->ParseStatRequests(input_document.GetRoot(), rs);
}
