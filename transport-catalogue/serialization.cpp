#include "serialization.h"

//Понимаю что можно все разделить по фонкциям, но я очень старался успеть к дедлайну

transport_catalogue_serialize::Color GetColor(svg::Color& col) {
    transport_catalogue_serialize::Color ser_col;
    if (std::holds_alternative<std::string>(col)) {
        transport_catalogue_serialize::ColorStr tmp;
        *tmp.mutable_color() = std::move(std::get<std::string>(col));
        *ser_col.mutable_collorstr() = std::move(tmp);
    }
    else if (std::holds_alternative<svg::Rgb>(col)) {
        transport_catalogue_serialize::Rgb rgb;
        rgb.set_red(std::get<svg::Rgb>(col).red);
        rgb.set_green(std::get<svg::Rgb>(col).green);
        rgb.set_blue(std::get<svg::Rgb>(col).blue);
        *ser_col.mutable_rgb() = rgb;
    }
    else if (std::holds_alternative<svg::Rgba>(col)) {
        transport_catalogue_serialize::Rgba rgba;
        rgba.set_red(std::get<svg::Rgba>(col).red);
        rgba.set_green(std::get<svg::Rgba>(col).green);
        rgba.set_blue(std::get<svg::Rgba>(col).blue);
        rgba.set_opacity(std::get<svg::Rgba>(col).opacity);
        *ser_col.mutable_rgba() = rgba;
    }

    return ser_col;
}

void Serialize(const std::filesystem::path& path, serialization_data::SerializationData&& s_data) {
    std::ofstream out_file(path, std::ios::binary);

    transport_catalogue_serialize::TransportCatalogue tc;

    for (auto& [id, name]  : s_data.name_repository) {
        transport_catalogue_serialize::ObjectName object_name;
        object_name.set_id(id);
        *object_name.mutable_name() = std::move(name);
        *tc.add_name_repository() = std::move(object_name);
    }

    for (auto& [name, coord, road_dist]  : s_data.stops) {
        transport_catalogue_serialize::Stop stop;
        stop.set_name(name);
        stop.set_latitude(coord.lat);
        stop.set_longitude(coord.lng);

        for (auto [name_stop_to, distances] : road_dist) {
            transport_catalogue_serialize::RoadDistances rd_s;
            rd_s.set_name_stop_to(name_stop_to);
            rd_s.set_distances(distances);

            *stop.add_road_distances() = rd_s;
        }
        *tc.add_stops() = std::move(stop);
    }

    for (auto [name, stops, is_roundtrip] : s_data.buses) {
        transport_catalogue_serialize::Bus bus;
        bus.set_name(name);
        bus.set_is_roundtrip(is_roundtrip);

        for (auto stop : stops) {
            bus.add_stops(stop);
        }
        *tc.add_buses() = std::move(bus);
    }

    transport_catalogue_serialize::SerializationSetting ss;

    *ss.mutable_transport_catalogue() = std::move(tc);

    transport_catalogue_serialize::VisualizationSettings vs;

    vs.set_width(s_data.vs.width);
    vs.set_height(s_data.vs.height);
    vs.set_padding(s_data.vs.padding);
    vs.set_line_width(s_data.vs.line_width);
    vs.set_stop_radius(s_data.vs.stop_radius);
    vs.set_underlayer_width(s_data.vs.underlayer_width);
    vs.set_bus_label_font_size(s_data.vs.bus_label_font_size);
    vs.set_stop_label_font_size(s_data.vs.stop_label_font_size);

    vs.add_bus_label_offset(s_data.vs.bus_label_offset.x);
    vs.add_bus_label_offset(s_data.vs.bus_label_offset.y);
    vs.add_stop_label_offset(s_data.vs.stop_label_offset.x);
    vs.add_stop_label_offset(s_data.vs.stop_label_offset.y);

    *vs.mutable_underlayer_color() = GetColor(s_data.vs.underlayer_color);

    for (auto& c : s_data.vs.color_palette) {
        *vs.add_color_palette() = std::move(GetColor(c));
    }

    *ss.mutable_vs() = std::move(vs);

    transport_catalogue_serialize::RouteSettings rs;

    rs.set_bus_wait_time(s_data.route_settings.bus_wait_time);
    rs.set_bus_velocity(s_data.route_settings.bus_velocity);

    *ss.mutable_rs() = std::move(rs);

    ss.SerializeToOstream(&out_file);
}

svg::Color DeserializeGetColor(transport_catalogue_serialize::Color& c) {
    svg::Color color;

    if (c.has_collorstr()) {
        transport_catalogue_serialize::ColorStr tmp = std::move(*c.mutable_collorstr());
        color = std::move(*tmp.mutable_color());
    }
    else if (c.has_rgb()) {
        color = svg::Rgb(c.rgb().red(), c.rgb().green(), c.rgb().blue());
    }
    else  if (c.has_rgba()) {
        color = svg::Rgba(c.rgba().red(), c.rgba().green(), c.rgba().blue(), c.rgba().opacity());
    }

    return color;
}

serialization_data::SerializationData Deserialize(const std::filesystem::path& path) {
    std::ifstream in_file(path, std::ios::binary);

    transport_catalogue_serialize::SerializationSetting ss;

    if (!ss.ParseFromIstream(&in_file)) {
        throw;
    }

    transport_catalogue_serialize::TransportCatalogue tc = std::move(*ss.mutable_transport_catalogue());

    serialization_data::SerializationData s_data;

    int size = tc.name_repository_size();
    s_data.name_repository.reserve(size);
    for (int i = 0; i < size; ++i) {
        transport_catalogue_serialize::ObjectName object_name = tc.name_repository(i);

        s_data.name_repository.push_back({object_name.id(), object_name.name()});
    }

    size = tc.stops_size();
    s_data.stops.reserve(size);
    for (int i = 0; i < size; ++i) {
        transport_catalogue_serialize::Stop stop = tc.stops(i);

        serialization_data::Stop stop_tmp;
        stop_tmp.name = stop.name();
        stop_tmp.coordinates = {stop.latitude(), stop.longitude()};

        int size_road = stop.road_distances_size();
        stop_tmp.road_distances.reserve(size_road);
        for (int j = 0; j < size_road; ++j) {
            transport_catalogue_serialize::RoadDistances rd = stop.road_distances(j);
            stop_tmp.road_distances.push_back({rd.name_stop_to(), rd.distances()});
        }
        s_data.stops.push_back(std::move(stop_tmp));
    }

    size = tc.buses_size();

    for (int i = 0; i < size; ++i) {
        transport_catalogue_serialize::Bus bus = tc.buses(i);

        serialization_data::Bus bus_tmp;
        bus_tmp.name = bus.name();
        bus_tmp.is_roundtrip = bus.is_roundtrip();

        int size_stops = bus.stops_size();
        bus_tmp.stops.reserve(size_stops);
        for (int j = 0; j < size_stops; ++j) {
            bus_tmp.stops.push_back(bus.stops(j));
        }

        s_data.buses.push_back(std::move(bus_tmp));
    }

    transport_catalogue_serialize::VisualizationSettings vs = std::move(*ss.mutable_vs());

    s_data.vs.width = vs.width();
    s_data.vs.height = vs.height();
    s_data.vs.padding = vs.padding();
    s_data.vs.line_width = vs.line_width();
    s_data.vs.stop_radius = vs.stop_radius();
    s_data.vs.underlayer_width = vs.underlayer_width();
    s_data.vs.bus_label_font_size = vs.bus_label_font_size();
    s_data.vs.stop_label_font_size = vs.stop_label_font_size();

    s_data.vs.bus_label_offset.x = vs.bus_label_offset(0);
    s_data.vs.bus_label_offset.y = vs.bus_label_offset(1);

    s_data.vs.stop_label_offset.x = vs.stop_label_offset(0);
    s_data.vs.stop_label_offset.y = vs.stop_label_offset(1);

    s_data.vs.underlayer_color = DeserializeGetColor(*vs.mutable_underlayer_color());

    size = vs.color_palette_size();

    for (int i = 0; i < size; ++i) {
        s_data.vs.color_palette.push_back(DeserializeGetColor(*vs.mutable_color_palette(i)));
    }

    transport_catalogue_serialize::RouteSettings rs = std::move(*ss.mutable_rs());

    s_data.route_settings.bus_velocity = rs.bus_velocity();
    s_data.route_settings.bus_wait_time = rs.bus_wait_time();

    return std::move(s_data);
}
