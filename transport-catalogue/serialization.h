#pragma once

#include "domain.h"
#include "geo.h"

#include <filesystem>
#include <fstream>
#include <transport_catalogue.pb.h>
#include <google/protobuf/message.h>



void Serialize(const std::filesystem::path& path, serialization_data::SerializationData&& s_data);

transport_catalogue_serialize::TransportCatalogue SerializeTransportCatalogue(serialization_data::SerializationData& s_data);
transport_catalogue_serialize::VisualizationSettings SerializeVisualizationSettings(renderer::VisualizationSettings& vs_data);
transport_catalogue_serialize::Color GetColor(svg::Color& col);

serialization_data::SerializationData Deserialize(const std::filesystem::path& path);

renderer::VisualizationSettings DeserializeVisualizationSettings(transport_catalogue_serialize::VisualizationSettings vs);
serialization_data::SerializationData DeserializeTransportCatalogue(transport_catalogue_serialize::TransportCatalogue& tc);
svg::Color DeserializeGetColor(transport_catalogue_serialize::Color& c);
