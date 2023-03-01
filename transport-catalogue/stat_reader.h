#pragma once

#include "transport_catalogue.h"

//чтение запросов на вывод и сам вывод

namespace transport_catalogue {

    namespace stat_reader {

        namespace detail {

            void OutBusInfo(std::string_view bus, TransportCatalogue& transport_catalogue);

            void OutStopInfo(std::string_view stop, TransportCatalogue& transport_catalogue);

            void ParseQuery(std::istream &input, TransportCatalogue& transport_catalogue);
        }

        void ReadQueryAndOutInfo(std::istream &input, TransportCatalogue& transport_catalogue);
    }
}
