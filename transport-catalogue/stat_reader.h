#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <string_view>

namespace transport_catalogue {

    namespace stat_reader {

        namespace detail {

            void OutBusInfo(std::ostream& out, std::string_view bus, TransportCatalogue& transport_catalogue);

            void OutStopInfo(std::ostream& out, std::string_view stop, TransportCatalogue& transport_catalogue);

            void ParseQuery(std::ostream& out, std::istream &input, TransportCatalogue& transport_catalogue);
        }

        void ReadQueryAndOutInfo(std::ostream& out, std::istream &input, TransportCatalogue& transport_catalogue);
    }
}
