#include "json_things.h"

#include <fstream>

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


auto pp::write_list_payload(const std::vector<star>& universe) -> void
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


auto pp::gen_thesquirrels_output(const std::vector<star>& universe) -> void
{
   nlohmann::json universe_json = nlohmann::json::array();
   for (const auto& star : universe)
   {
      nlohmann::json system_json = star;
      system_json["planets"] = nlohmann::json::array();
      for (const auto& planet : star.m_planets)
      {
         // nlohmann::json planets = nlohmann::json::array();
         nlohmann::json moons = nlohmann::json::array();
         for (const auto& moon : planet.m_moons)
         {
            moons.push_back(moon);
         }
         nlohmann::json planet_json = planet;
         planet_json["moons"] = moons;
         system_json["planets"].push_back(planet_json);
      }
      universe_json.push_back(system_json);
   }
   std::ofstream o("universe.json");
   o << std::setw(4) << universe_json << std::endl;
}


auto pp::gen_web_output(const std::vector<star>& universe) -> void
{
   std::ifstream f_label_shifts("label_shifts.json");
   nlohmann::json shift_data = nlohmann::json::parse(f_label_shifts);

   nlohmann::json universe_json;
   for (const auto& star : universe)
   {
      nlohmann::json system_json = star;
      system_json["planets"] = nlohmann::json::array();
      system_json["position"] = nlohmann::json::array();
      system_json["position"].push_back(star.m_x);
      system_json["position"].push_back(star.m_y);
      system_json["position"].push_back(star.m_z);

      system_json["extra_classes"] = nlohmann::json::array();
      if (shift_data.contains(star.m_name))
         system_json["extra_classes"].push_back(shift_data[star.m_name]);
      for (const auto& planet : star.m_planets)
      {
         nlohmann::json moons = nlohmann::json::array();
         for (const auto& moon : planet.m_moons)
         {
            moons.push_back(moon);
         }
         nlohmann::json planet_json = planet;
         planet_json["moons"] = moons;
         system_json["planets"].push_back(planet_json);
      }
      universe_json[star.m_name] = system_json;
   }
   std::ofstream o("web_data_debugging.json");
   o << std::setw(4) << universe_json << std::endl;


   const auto json_str = universe_json.dump();
   write_binary_file("data", compress(json_str));
}


