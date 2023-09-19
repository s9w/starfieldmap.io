#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <map>
#include <variant>
#include <vector>
#include <regex>
#include <chrono>
#include <ranges>

#include <cppp/tools.h>


namespace pp
{
   template<typename T>
   using formid_map = std::unordered_map<formid, T>;
   

   template<typename T>
   concept fillable = requires(T t, const line_content& lc)
   {
      {t.process_line(lc)} -> std::same_as<void>;
   };

   auto get_file_str(const std::string_view& filename) -> std::string
   {
      std::ifstream t(filename.data());
      t.seekg(0, std::ios::end);
      const ptrdiff_t size = t.tellg();
      std::string result;
      result.resize(size);
      t.seekg(0);
      t.read(result.data(), size);
      return result;
   }

   struct file_chopper{
      std::string m_file_content;
      std::vector<line_content> m_lines;
      explicit file_chopper(const std::string_view& filename)
         : m_file_content(get_file_str(filename))
      {
         m_lines.reserve(m_file_content.size() / 20);
         ptrdiff_t last_newline_pos = -1;
         int line_number = 1;
         while (true)
         {
            const auto next_newline_pos = m_file_content.find('\n', last_newline_pos + 1);
            if (next_newline_pos == std::string::npos)
               break;
            std::string_view content(m_file_content.data() + last_newline_pos + 1, m_file_content.data() + next_newline_pos);
            const auto level = get_indentation_level(content);
            content.remove_prefix(static_cast<size_t>(level) * 2);
            m_lines.emplace_back(
               line_content{
                  .m_level = level,
                  .m_line_content = content,
                  .m_line_number = line_number
               }
            );
            last_newline_pos = static_cast<ptrdiff_t>(next_newline_pos);
            ++line_number;
         }
         m_lines.shrink_to_fit();
      }
   };

   template<fillable T>
   auto run(const std::string_view& filename, const std::string_view name, formid_map<T>& map) -> void
   {
      const file_chopper chopper(filename);
      std::optional<T> next_obj;
      bool looking_for_first_formid = true;
      const std::string name_starter = std::format("FormID: {}", name);
      for(const line_content&content : chopper.m_lines)
      {
         
         if (content.m_line_content.starts_with(name_starter))
         {
            if(looking_for_first_formid == false)
            {
               if(next_obj.value().reject() == false)
                  map.emplace(next_obj.value().m_formid, next_obj.value());
            }
            else
            {
               looking_for_first_formid = false;
            }
            next_obj.emplace(get_formid(content.m_line_content));
         }
         else if(looking_for_first_formid == false)
         {
            next_obj.value().process_line(content);
         }
      }
      if (next_obj.value().reject() == false)
         map.emplace(next_obj.value().m_formid, next_obj.value());
   }


   enum class prop_function_type{not_set, mul_add, add, set, rem};

   struct property {
      prop_function_type m_function_type = prop_function_type::not_set;
      std::string m_property_name;
      float m_mult;
      float m_add;
      float m_step;

      friend bool operator==(property const& lhs, property const& rhs) = default;
   };


   struct lctn {
      formid m_formid;
      int m_system_level{};
      std::string m_name;
      bool m_reject = true;

      explicit lctn(const formid formid)
         : m_formid(formid)
      {

      }
      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "System level", m_system_level);
         extract(line.m_line_content, "FULL - Name", m_name);

         std::string m_parent_location{};
         if (extract(line.m_line_content, "PNAM - Parent Location", m_parent_location) && get_formid(m_parent_location) == static_cast<formid>(0x0001a53a))
         {
            m_reject = false;
         }
      }

      [[nodiscard]] auto reject() const -> bool
      {
         return m_reject;
      }
   };

   

   struct stdt {
      formid m_formid;
      std::string m_name;
      float m_x{};
      float m_y{};
      float m_z{};
      bool m_reject = true;
      int m_star_id{};

      explicit stdt(const formid formid)
         : m_formid(formid)
      {
      }
      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);
         extract(line.m_line_content, "DNAM - Star ID", m_star_id);
         extract(line.m_line_content, "x", m_x);
         extract(line.m_line_content, "y", m_y);
         extract(line.m_line_content, "z", m_z);

         if (m_name.empty() == false)
            m_reject = false;
      }

      [[nodiscard]] auto reject() const -> bool
      {
         return m_reject;
      }
   };
   

   struct planet_biome{
      float m_percentage{};
      formid m_biome_ref{};
      formid m_resource_gen_override{};
   };
   auto operator==(const planet_biome& a, const planet_biome& b) -> bool
   {
      return a.m_biome_ref == b.m_biome_ref;
   }

   struct animal{
      formid m_formid;
   };
   auto operator==(const animal& a, const animal& b) -> bool
   {
      return a.m_formid == b.m_formid;
   }
   struct plant{
      formid m_formid;
   };
   auto operator==(const plant& a, const plant& b) -> bool
   {
      return a.m_formid == b.m_formid;
   }

   enum class body_type{unset, planet, moon};
   struct pndt {
      formid m_formid{};
      bool m_reject = true;
      list_item_detector m_biome_detector;
      list_item_detector m_animal_detector;
      list_item_detector m_flora_detector;

      std::string m_name;
      float m_gravity;
      float m_temperature{};
      int m_star_id{};
      int m_planet_id{};
      int m_primary_planet_id{};
      body_type m_body_type = body_type::unset;
      std::vector<planet_biome> m_biome_refs;
      std::vector<animal> m_animal_refs;
      std::vector<plant> m_flora_refs;

      explicit pndt(const formid formid)
         : m_formid(formid)
         , m_biome_detector("PPBD - Biome #")
         , m_animal_detector("Fauna #")
         , m_flora_detector("Flora #")
      {

      }


      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);
         extract(line.m_line_content, "TEMP - Temperature in C", m_temperature);
         extract(line.m_line_content, "Star ID", m_star_id);
         extract(line.m_line_content, "Planet ID", m_planet_id);
         extract(line.m_line_content, "Primary planet ID", m_primary_planet_id);
         extract(line.m_line_content, "Gravity", m_gravity);

         const auto biome_generator = [](const std::vector<std::string_view>& lines) -> std::optional<planet_biome> {
            planet_biome biome;
            for (const auto& l : lines)
            {
               extract(l, "Percentage", biome.m_percentage);
               extract(l, "Biome reference", biome.m_biome_ref);
               extract(l, "Resource gen override", biome.m_resource_gen_override);
            }
            return biome;
         };
         m_biome_detector.process_line(line, m_biome_refs, biome_generator);

         const auto animal_generator = [](const std::vector<std::string_view>& lines) -> std::optional<animal> {
            pp_assert(lines.size() == 1);
            return animal{.m_formid = get_formid(lines[0])};
            };
         m_animal_detector.process_line(line, m_animal_refs, animal_generator);

         const auto flora_generator = [](const std::vector<std::string_view>& lines) -> std::optional<plant> {
            std::string str;
            plant result{};
            for(const auto& l : lines)
            {
               if(extract(l, "Model", str))
               {
                  result.m_formid = get_formid(l);
               }
            }
            return result;
            };
         m_flora_detector.process_line(line, m_flora_refs, flora_generator);

         std::string body_type_str;
         if(extract(line.m_line_content, "CNAM - Body type", body_type_str))
         {
            if (body_type_str == "Moon")
            {
               m_body_type = body_type::moon;
               m_reject = false;
            }
            else if (body_type_str == "Planet")
            {
               m_body_type = body_type::planet;
               m_reject = false;
            }
         }

      }

      [[nodiscard]] auto reject() const -> bool
      {
         return m_reject;
      }
   };


   struct omod {
      formid m_formid;
      std::optional<std::string> m_name;
      std::vector<property> m_properties;
      list_item_detector m_property_builder;

      explicit omod(const formid formid)
         : m_formid(formid)
         , m_property_builder("Property #")
      {
      }


      [[nodiscard]] auto reject() const -> bool
      {
         return false;
      }

      static auto build_property(const std::vector<std::string_view>& lines) -> std::optional<property>
      {
         const std::regex pattern("Value (\\d) - (.+?): (.+)");
         std::string property_name;
         float value1{};
         float value2{};
         prop_function_type fun_type = prop_function_type::not_set;
         float step{};
         for(const auto& line : lines)
         {
            std::string fun_type_str;
            if(extract(line, "Function Type", fun_type_str))
            {
               if (fun_type_str == "SET") fun_type = prop_function_type::set;
               if (fun_type_str == "ADD") fun_type = prop_function_type::add;
               if (fun_type_str == "MUL+ADD") fun_type = prop_function_type::mul_add;
               if (fun_type_str == "REM") fun_type = prop_function_type::rem;
            }
            extract(line, "Property Name", property_name);
            extract(line, "Step", step);


            std::smatch match;
            std::string line_str(line);
            if(line.starts_with("Value ") && std::regex_match(line_str, match, pattern)) // TODO: wow this is atrocious
            {
               if(match[2] == "FormID")
               {
                  return std::nullopt;
               }
               float value{};
               if(match[2] == "Float")
                  value = std::stof(match[3]);
               else if (match[2] == "Int")
                  value = static_cast<float>(std::stoi(match[3]));

               if(match[1] == "1")
                  value1 = value;
               else if (match[1] == "2")
                  value2 = value;
            }
         }
         if (fun_type == prop_function_type::not_set)
            std::terminate();
         return(
            property{
               .m_function_type = fun_type,
               .m_property_name = property_name,
               .m_mult = value1,
               .m_add = value2,
               .m_step = step
            }
         );
      }

      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);

         m_property_builder.process_line(line, m_properties, omod::build_property);
      }
   };

   struct moon{
      std::string m_name;
      std::vector<animal> m_fauna;
      std::vector<plant> m_flora;
      float m_temperature{};
      float m_gravity{};
   };

   struct planet{
      std::string m_name;
      float m_temperature{};
      float m_gravity{};
      std::vector<animal> m_fauna;
      std::vector<plant> m_flora;
      std::vector<moon> m_moons;
      int m_planet_id{};
   };

   struct star {
      float m_x{};
      float m_y{};
      float m_z{};
      int m_level{};
      std::string m_name;
      std::vector<planet> m_planets;
   };


   auto build_universe(
      const formid_map<pp::lctn>& lctns,
      const formid_map<pp::stdt>& stdts,
      const formid_map<pp::pndt>& pndts
   ) -> std::unordered_map<int, star>
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
               .m_x = value.m_x,
               .m_y = value.m_y,
               .m_z = value.m_z,
               .m_level = lctn.m_system_level,
               .m_name = value.m_name
            }
         );
      }

      // Prep planets
      std::unordered_map<int, std::vector<planet>> planets_by_starid;
      for (const auto& value : pndts | std::views::values)
      {
         if (value.m_body_type != body_type::planet)
            continue;
         planets_by_starid[value.m_star_id].emplace_back(
            planet{
               .m_name = value.m_name,
               .m_temperature = value.m_temperature,
               .m_gravity = value.m_gravity,
               .m_fauna = value.m_animal_refs,
               .m_flora = value.m_flora_refs,
               .m_planet_id = value.m_planet_id
            }
         );
      }

      // Add moons to planets
      for (const auto& value : pndts | std::views::values)
      {
         if (value.m_body_type != body_type::moon)
            continue;
         auto& planets = planets_by_starid.at(value.m_star_id);
         for (auto& planet : planets)
         {
            if (planet.m_planet_id != value.m_primary_planet_id)
               continue;
            planet.m_moons.push_back(
               moon{
                  .m_name = value.m_name,
                  .m_fauna = value.m_animal_refs,
                  .m_flora = value.m_flora_refs,
                  .m_temperature = value.m_temperature,
                  .m_gravity = value.m_gravity
               }
            );
         }
      }

      // Add planets to stars
      for (const auto& [starid, value] : planets_by_starid)
      {
         starid_to_star.at(starid).m_planets = value;
      }

      return starid_to_star;
   }

} // namespace pp




auto main() -> int
{
   using namespace pp;
   std::optional<ms_timer> timer;
   timer.emplace();

   formid_map<pp::lctn> lctns;
   formid_map<pp::omod> omods;
   formid_map<pp::stdt> stdts;
   formid_map<pp::pndt> pndts;
   std::vector<std::jthread> threads;
   threads.reserve(10);
   threads.emplace_back(pp::run<pp::lctn>, "../../data/xdump_lctn.txt", "LCTN", std::ref(lctns));
   threads.emplace_back(pp::run<pp::stdt>, "../../data/xdump_stars.txt", "STDT", std::ref(stdts));
   threads.emplace_back(pp::run<pp::pndt>, "../../data/xdump_planets.txt", "PNDT", std::ref(pndts));
   threads.emplace_back(pp::run<pp::omod>, "../../data/xdump_omod.txt", "OMOD", std::ref(omods));
   threads.clear();
   timer.emplace();

   const std::unordered_map<int, star> universe = build_universe(lctns, stdts, pndts);
   [[maybe_unused]] const auto& narion = universe.at(88327);
   timer.reset();

   [[maybe_unused]] int end = 0;
}

