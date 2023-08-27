#include "transport_catalogue.h"
#include "serialization.h"
#include "json_reader.h"
#include "map_renderer.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        transport_catalogue::TransportCatalogue db;
        renderer::MapRenderer mr;

        JsonReader json_reader(db, mr);

        auto [serialization_setting, serialization_data] = json_reader.ParseJSONtoGetDataForSerialization(std::cin);
        Serialize(serialization_setting,  std::move(serialization_data));

    } else if (mode == "process_requests"sv) {

        transport_catalogue::TransportCatalogue db;
        renderer::MapRenderer mr;

        JsonReader json_reader(db, mr);
        json_reader.ParseJsonProcessRequests(std::cin);
        json_reader.OutStatRequests(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
