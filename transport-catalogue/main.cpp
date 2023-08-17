#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

#include <iostream>
#include <iomanip>

int main() {
    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer mr;

    JsonReader json_reader(db, mr);
    json_reader.ParseJSON(std::cin);

    std::cout << std::setprecision(6);
    
    json_reader.OutStatRequests(std::cout);

    return 0;
}
