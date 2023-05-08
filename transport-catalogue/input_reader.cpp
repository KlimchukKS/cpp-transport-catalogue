#include "input_reader.h"
#include "transport_catalogue.h"

#include <algorithm>

namespace transport_catalogue {

    namespace input_reader {

        namespace detail {

            using namespace std::string_literals;

            std::string ReadLine(std::istream &input) {
                std::string s;
                std::getline(input, s);
                return s;
            }

            std::pair<RequestsType, std::string> ParseQueryLine(std::string&& query) {
                auto pos = query.find_first_of(" ");
                if (query.substr(0, pos) == "Stop"s) {
                    return {RequestsType::kStop,
                            query.substr(query.find_first_not_of(" ", pos), query.size())};
                }
                if (query.substr(0, pos) == "Bus"s) {
                    return {RequestsType::kBus,
                            query.substr(query.find_first_not_of(" ", pos), query.size())};
                }
                return {RequestsType::kError, query};
            }

            Queries RequestQueue(std::istream& input) {
                int number_requests;
                input >> number_requests;
                ReadLine(input);

                std::vector<std::string> requests_add_stop;
                std::vector<std::string> requests_add_bus;

                for (int i = 0; i < number_requests; ++i) {
                    std::string query = ReadLine(input);
                    auto [request_type, str] = std::move(ParseQueryLine(move(query)));
                    switch (request_type) {
                        case RequestsType::kStop:
                            requests_add_stop.push_back(move(str));
                            break;
                        case RequestsType::kBus:
                            requests_add_bus.push_back(move(str));
                            break;
                        case RequestsType::kError:
                            throw (std::logic_error("Incorrect query: "s + str));
                    }
                }
                return {requests_add_stop, requests_add_bus};
            }

            QueryStops ParseQueryStops(std::vector<std::string>&& query_stops) {
                std::vector<std::tuple<std::string, double, double>> stop_lat_lng;
                std::vector<std::tuple<std::string, int, std::string>> stop_distance_to_stop;

                for (auto& query  : query_stops) {
                    auto pos_begin = query.find_first_of(":");

                    std::string stop = query.substr(0u, pos_begin);

                    pos_begin = query.find_first_not_of(" ", pos_begin + 1u);

                    auto pos_end = query.find_first_of(",", pos_begin);

                    double lat = std::stod(query.substr(pos_begin, pos_end), &pos_begin);
                    pos_begin = query.find_first_not_of(" ", pos_end + 1u);
                    pos_end = query.find_first_of(",", pos_begin + 1u);

                    double lng = std::stod(query.substr(pos_begin, pos_end));

                    stop_lat_lng.push_back({stop, lat, lng});

                    while (pos_end != std::string::npos) {
                        pos_begin = query.find_first_not_of(" ", pos_end + 1u);
                        pos_end = query.find_first_of("m", pos_begin);

                        int distance = std::stoi(query.substr(pos_begin, pos_end - pos_begin));

                        pos_begin = query.find_first_of("o", pos_end + 1u);
                        pos_begin = query.find_first_not_of(" ", pos_begin + 1u);

                        pos_end = query.find_first_of(",", pos_begin);

                        std::string to_stop = query.substr(pos_begin,
                                                           (pos_end == std::string::npos) ?
                                                           std::string::npos : pos_end - pos_begin);

                        stop_distance_to_stop.push_back({stop, distance, to_stop});
                    }
                }
                return {stop_lat_lng, stop_distance_to_stop};
            }

            QueryBuses ParseQueryBuses(std::vector<std::string>&& query_bus) {
                QueryBuses bus_stops;
                bus_stops.reserve(query_bus.size());
                for (std::string_view query  : query_bus) {
                    query.remove_prefix(std::min(query.find_first_not_of(" "), query.size()));
                    auto pos_begin = query.find_first_of(":");
                    auto bus = query.substr(0, pos_begin);
                    query.remove_prefix(pos_begin + 1);
                    char ch = (query.find_first_of(">") != std::string::npos) ? '>' : '-';
                    size_t tmp = 0;
                    std::vector<std::string> stops;
                    while (tmp != std::string::npos) {
                        query.remove_prefix(std::min(query.find_first_not_of(" "), query.size()));
                        tmp = query.find(ch);
                        std::string_view stop = query.substr(0, tmp);
                        query.remove_prefix(tmp + 1);
                        while (stop[stop.size() - 1] == ' ') {
                            stop.remove_suffix(1);
                        }
                        stops.push_back(std::string (stop));
                    }
                    if (ch == '-') {
                        std::vector<std::string> tmp = {stops.begin(), stops.end() - 1};
                        tmp.insert(tmp.end(), stops.rbegin(), stops.rend());
                        bus_stops.push_back({std::string(bus), move(tmp)});
                        continue;
                    }
                    bus_stops.push_back({std::string(bus), move(stops)});
                }
                return bus_stops;
            }
        }

        void ParseQuery(std::istream &input, TransportCatalogue& transport_catalogue) {
            auto [query_stops, query_bus] = move(detail::RequestQueue(input));

            auto [stop_lat_lng, stop_distance_to_stop] = std::move(detail::ParseQueryStops(move(query_stops)));
            for (auto& stop : stop_lat_lng) {
                transport_catalogue.AddStop(stop);
            }
            for (auto& stop : stop_distance_to_stop) {
                transport_catalogue.SetDistanceBetweenStops(stop);
            }

            for (auto& [bus, stops] : detail::ParseQueryBuses(move(query_bus))) {
                transport_catalogue.AddBus(bus, stops, false);
            }
        }
    }
}
