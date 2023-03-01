#pragma once

#include "transport_catalogue.h"

#include <string>
#include <istream>
#include <vector>
#include <tuple>

namespace transport_catalogue {

    namespace input_reader {

        namespace detail {
            using Queries = std::pair<std::vector<std::string>, std::vector<std::string>>;
            using QueryStops = std::pair<std::vector<std::tuple<std::string, double, double>>,
                    std::vector<std::tuple<std::string, int, std::string>>>;
            using QueryBuses = std::vector<std::pair<std::string, std::vector<std::string>>>;

            enum class RequestsType {
                kStop,
                kBus,
                kError,
            };

            std::string ReadLine(std::istream &input);

            std::pair<RequestsType, std::string> ParseQueryLine(std::string &&query);

            Queries RequestQueue(std::istream &input);

            QueryStops ParseQueryStops(std::vector<std::string>&& query_stops);
            QueryBuses ParseQueryBuses(std::vector<std::string>&& query_bus);
        }

        void ParseQuery(std::istream &input, TransportCatalogue& transport_catalogue);
    }
}
