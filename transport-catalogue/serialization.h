#pragma once

#include "domain.h"
#include "geo.h"

#include <filesystem>
#include <fstream>
#include <transport_catalogue.pb.h>
#include <google/protobuf/message.h>



void Serialize(const std::filesystem::path& path, serialization_data::SerializationData&& s_data);

serialization_data::SerializationData Deserialize(const std::filesystem::path& path);
