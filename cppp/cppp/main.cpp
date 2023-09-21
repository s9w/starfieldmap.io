#include <algorithm>
#include <string>
#include <optional>
#include <map>
#include <vector>
#include <chrono>
#include <ranges>

#include <cppp/json_things.h>
#include <cppp/tools.h>
#include <cppp/xedit_parsing.h>


namespace pp
{

   auto get_biomes(const std::vector<planet_biome_ref>& refs, const formid_map<pp::biom>& bioms) -> std::vector<biome>
   {
      std::vector<biome> result;
      result.reserve(refs.size());
      for(const auto& ref : refs)
      {
         result.push_back(
            biome{
               .m_percentage = ref.m_percentage,
               .m_name = bioms.at(ref.m_biome_ref).m_name,
               .m_formid = ref.m_biome_ref
            }
         );
      }

      // Sort by most common first
      const auto pred = [](const biome& a, const biome& b){
         return a.m_percentage > b.m_percentage;
      };
      std::ranges::sort(result, pred);

      return result;
   }

   auto get_plants(const std::vector<formid>& refs, const formid_map<pp::flor>& flors) -> std::vector<flora>
   {
      std::vector<flora> result;
      result.reserve(refs.size());
      for (const auto& ref : refs)
      {
         result.push_back(
            flora{
               .m_name = flors.at(ref).m_name,
               .m_formid = ref
            }
         );
      }

      return result;
   }

   auto build_universe(
      const formid_map<pp::lctn>& lctns,
      const formid_map<pp::stdt>& stdts,
      const formid_map<pp::pndt>& pndts,
      const formid_map<pp::biom>& bioms,
      const formid_map<pp::flor>& flors
   ) -> std::vector<star>
   {
      // Combine star-LCTN and STDT
      std::map<std::string, formid> name_to_lctn;
      for (const auto& [key, value] : lctns)
         name_to_lctn.emplace(value.m_name, key);

      std::unordered_map<int, star> starid_to_star;
      for (const auto& value : stdts | std::views::values)
      {
         const auto& lctn_formid = name_to_lctn.at(value.m_name);
         const auto& lctn = lctns.at(lctn_formid);
         starid_to_star.emplace(
            value.m_star_id,
            star{
               .m_formid = value.m_formid,
               .m_x = value.m_x,
               .m_y = value.m_y,
               .m_z = value.m_z,
               .m_level = lctn.m_system_level,
               .m_name = value.m_name
            }
         );
      }

      constexpr auto moons = std::views::filter([](const pndt& el) {return el.m_body_type == body_type::moon; });
      constexpr auto planets = std::views::filter([](const pndt& el) {return el.m_body_type == body_type::planet; });

      auto get_body = [&](const pndt& value){
         return body{
            .m_formid = value.m_formid,
            .m_name = value.m_name,
            .m_temperature = value.m_temperature,
            .m_temp_level = value.m_temp_level,
            .m_gravity = std::format("{:.2f}", value.m_gravity),
            .m_fauna_count = static_cast<int>(value.m_animals.size()),
            .m_flora = get_plants(value.m_plants, flors),
            .m_oxygen_amount = value.m_oxygen_amount,
            .m_biomes = get_biomes(value.m_biome_refs, bioms),
            .m_traits = value.m_traits
         };
      };

      // Prep planets
      std::unordered_map<int, std::vector<planet>> planets_by_starid;
      for (const auto& value : pndts | std::views::values | planets)
      {
         planets_by_starid[value.m_star_id].emplace_back(
            planet{
               get_body(value),
               {},
               value.m_planet_id
            }
         );
      }
      

      // Add moons to planets
      for (const auto& value : pndts | std::views::values | moons)
      {
         auto& system_planets = planets_by_starid.at(value.m_star_id);
         for (auto& planet : system_planets)
         {
            if (planet.m_planet_id != value.m_primary_planet_id)
               continue;
            planet.m_moons.push_back(
               get_body(value)
            );
         }
      }

      // Add planets to stars
      for (const auto& [starid, value] : planets_by_starid)
      {
         starid_to_star.at(starid).m_planets = value;
      }

      std::vector<star> result;
      result.reserve(starid_to_star.size());
      for (const auto& star : starid_to_star | std::views::values)
         result.push_back(star);
      return result;
   }

} // namespace pp




auto main() -> int
{
   using namespace pp;
   std::optional<ms_timer> timer;
   timer.emplace("xedit parsing");

   formid_map<pp::lctn> lctns;
   formid_map<pp::omod> omods;
   formid_map<pp::stdt> stdts;
   formid_map<pp::pndt> pndts;
   formid_map<pp::biom> bioms;
   formid_map<pp::flor> flors;
   std::vector<std::jthread> threads;
   threads.reserve(10);
   threads.emplace_back(pp::parse_xedit_output<pp::lctn>, "../../data/xdump_lctn.txt", "LCTN", std::ref(lctns));
   threads.emplace_back(pp::parse_xedit_output<pp::stdt>, "../../data/xdump_stars.txt", "STDT", std::ref(stdts));
   threads.emplace_back(pp::parse_xedit_output<pp::pndt>, "../../data/xdump_planets.txt", "PNDT", std::ref(pndts));
   threads.emplace_back(pp::parse_xedit_output<pp::omod>, "../../data/xdump_omod.txt", "OMOD", std::ref(omods));
   threads.emplace_back(pp::parse_xedit_output<pp::biom>, "../../data/xdump_biomes.txt", "BIOM", std::ref(bioms));
   threads.emplace_back(pp::parse_xedit_output<pp::flor>, "../../data/xdump_flora.txt", "FLOR", std::ref(flors));
   threads.clear();

   timer.emplace("universe building");
   const std::vector<star> universe = build_universe(lctns, stdts, pndts, bioms, flors);

   timer.emplace("squirrel output");
   gen_thesquirrels_output(universe);

   timer.emplace("gen_web_output()");
   gen_web_output(universe);


   timer.emplace("write_list_payload()");
   write_list_payload(universe);

   [[maybe_unused]] int end = 0;
}

