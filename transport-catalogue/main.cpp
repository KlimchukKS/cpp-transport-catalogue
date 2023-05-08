#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

int main() {

    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer mr;

    JsonReader json_reader(db, mr);
    json_reader.ParseJSON(std::cin);
    json_reader.OutStatRequests(std::cout);

    return 0;
}
