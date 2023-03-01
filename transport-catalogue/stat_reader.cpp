#include "stat_reader.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <string_view>
#include <algorithm>
#include <unordered_set>

namespace transport_catalogue {

    namespace stat_reader {

        using namespace std::string_literals;
        using namespace std::string_view_literals;

        namespace detail {
            void OutBusInfo(std::string_view bus, TransportCatalogue& transport_catalogue) {
                try {
                    auto [bus_name,
                            stops_route,
                            unique_stops,
                            route_length,
                            curvature] = transport_catalogue.GetBusInfo(bus);
                    std::cout << "Bus "s <<bus_name << ": "s
                              << stops_route << " stops on route, "s
                              << unique_stops << " unique stops, "s
                              << route_length << " route length, "s
                              << std::setprecision(6)
                              << curvature << " curvature\n";
                } catch (const std::logic_error& error) {
                    std::cout << "Bus " << bus << ": not found\n";
                }
            }

            void OutStopInfo(std::string_view stop, TransportCatalogue& transport_catalogue) {
                try {
                    std::unordered_set<std::string_view> buses = transport_catalogue.GetStopInfo(stop);
                    if (buses.empty()) {
                        std::cout << "Stop " << stop << ": no buses\n";
                    } else {
                        std::cout << "Stop " << stop << ": buses ";
                        std::vector<std::string_view> tmp(buses.size());
                        std::copy(buses.begin(), buses.end(), tmp.begin());
                        std::sort(tmp.begin(), tmp.end());
                        for (auto& bus : tmp) {
                            std::cout << bus << " ";
                        }
                        std::cout << "\n";
                    }
                } catch (const std::logic_error& error) {
                    std::cout << "Stop " << stop << ": not found\n";
                }
            }

            void ParseQuery(std::istream &input, TransportCatalogue& transport_catalogue) {
                using namespace std::string_literals;
                int number_requests;
                input >> number_requests;

                std::string line;
                std::getline(input, line);

                for (int i = 0; i < number_requests; ++i) {
                    std::getline(input, line);
                    std::string_view query = line;

                    query.remove_prefix(std::min(query.find_first_not_of(" "), query.size()));

                    std::string_view command = query.substr(0u, query.find_first_of(" "));

                    query.remove_prefix(std::min(query.find_first_of(" "), query.size()));
                    query.remove_prefix(std::min(query.find_first_not_of(" "), query.size()));

                    if (command == "Bus"sv) {
                        OutBusInfo(query, transport_catalogue);
                    } else if (command == "Stop"sv) {
                        OutStopInfo(query, transport_catalogue);
                    }
                }
            }
        }

        void ReadQueryAndOutInfo(std::istream &input, TransportCatalogue& transport_catalogue) {
            detail::ParseQuery(input, transport_catalogue);
        }
    }
}
