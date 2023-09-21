#include "list_output.h"

#include <cppp/tools.h>

#include <nlohmann/json.hpp>

namespace
{
   using namespace pp;



   
} // namespace {}


void pp::to_json(nlohmann::json& j, const formid& p) {
   j = as_big(p);
}
void pp::to_json(nlohmann::json& j, const biome& p) {
   j["percentage"] = p.m_percentage;
   j["name"] = p.m_name;
   j["formid"] = as_big(p.m_formid);
}
void pp::to_json(nlohmann::json& j, const flora& p) {
   j["name"] = p.m_name;
   j["formid"] = as_big(p.m_formid);
}
void pp::to_json(nlohmann::json& j, const body& p) {
   j["name"] = p.m_name;
   j["biomes"] = p.m_biomes;
   j["fauna_count"] = p.m_fauna_count;
   j["flora"] = p.m_flora;
   j["formid"] = as_big(p.m_formid);
   j["oxygen_amount"] = p.m_oxygen_amount;
   j["temperature"] = p.m_temperature;
   j["temp_level"] = p.m_temp_level;
   j["gravity"] = p.m_gravity;
   j["traits"] = p.m_traits;
}
void pp::to_json(nlohmann::json& j, const star& p) {
   j["name"] = p.m_name;
   j["formid"] = as_big(p.m_formid);
   j["level"] = p.m_level;
   j["planet_count"] = static_cast<int>(p.m_planets.size());
   int moon_count = 0;
   for (const auto& planet : p.m_planets)
      moon_count += static_cast<int>(planet.m_moons.size());
   j["moon_count"] = moon_count;
}


auto pp::write_list(const std::vector<star>& universe) -> void
{
   nlohmann::json template_data;
   template_data["bodies"] = nlohmann::json::array();
   for (const auto& star : universe)
   {
      for (const auto& planet : star.m_planets)
      {
         nlohmann::json planet_template_data = static_cast<body>(planet);
         planet_template_data["class"] = "planet";

         planet_template_data["nav"] = nlohmann::json::array();
         planet_template_data["nav"].emplace_back(std::format("{} system (Level {})", star.m_name, star.m_level));
         planet_template_data["nav"].emplace_back(planet.m_name);

         template_data["bodies"].push_back(planet_template_data);
         for (const auto& moon : planet.m_moons)
         {
            nlohmann::json moon_template_data = static_cast<body>(moon);
            moon_template_data["class"] = "moon";

            moon_template_data["nav"] = nlohmann::json::array();
            moon_template_data["nav"].emplace_back(std::format("{} system (Level {})", star.m_name, star.m_level));
            moon_template_data["nav"].emplace_back(planet.m_name);
            moon_template_data["nav"].emplace_back(moon.m_name);

            template_data["bodies"].push_back(moon_template_data);
         }
      }
   }
   const auto json_str = template_data.dump();
   write_binary_file("binary_payload", compress(json_str));
}
