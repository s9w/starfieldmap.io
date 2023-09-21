#pragma once

#include <cppp/tools.h>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace pp
{

   void to_json(nlohmann::json& j, const formid& p);
   void to_json(nlohmann::json& j, const biome& p);
   void to_json(nlohmann::json& j, const flora& p);
   void to_json(nlohmann::json& j, const body& p);
   void to_json(nlohmann::json& j, const star& p);

   auto write_list(const std::vector<star>& universe) -> void;
}
