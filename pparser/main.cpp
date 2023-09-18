#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <map>
#include <variant>
#include <vector>
#include <regex>
#include <chrono>

#include "fundamentals.h"
#include "tools.h"


namespace pp
{

   

   template<typename T>
   concept fillable = requires(T t, const line_content& lc)
   {
      {t.process_line(lc)} -> std::same_as<void>;
   };


   template<fillable T>
   auto run(const std::string& filename, const std::string_view name, std::map<uint32_t, T>& map) -> void
   {
      std::ifstream f(filename);
      std::string line;
      int line_number = 1;
      std::optional<T> next_obj;
      bool looking_for_first_formid = true;
      const std::string name_starter = std::format("FormID: {}", name);
      while (std::getline(f, line))
      {
         const line_content content = get_line_content(std::move(line), line_number);

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
         ++line_number;
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
   };


   struct lctn {
      uint32_t m_formid;
      int m_system_level{};
      std::string m_name;
      bool m_reject = true;

      explicit lctn(const uint32_t formid)
         : m_formid(formid)
      {

      }
      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "System level", m_system_level);
         extract(line.m_line_content, "FULL - Name", m_name);

         std::string m_parent_location{};
         if (extract(line.m_line_content, "PNAM - Parent Location", m_parent_location) && get_formid(m_parent_location) == 0x0001a53a)
         {
            m_reject = false;
         }
      }

      [[nodiscard]] auto reject() const -> bool
      {
         return m_reject;
      }
   };

   struct star {
      uint32_t m_formid;
      std::string m_name;
      float m_x{};
      float m_y{};
      float m_z{};

      explicit star(const uint32_t formid)
         : m_formid(formid)
      {
      }
      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);
         extract(line.m_line_content, "x", m_x);
         extract(line.m_line_content, "y", m_y);
         extract(line.m_line_content, "z", m_z);
      }

      [[nodiscard]] auto reject() const -> bool
      {
         return false;
      }
   };
   

   struct planet_biome{
      float m_percentage{};
      uint32_t m_biome_ref{};
      uint32_t m_resource_gen_override{};
   };

   enum class body_type{unset, planet, moon};
   struct planet {
      uint32_t m_formid{};
      bool m_reject = true;
      list_item_detector m_list_item_detector;

      std::string m_name;
      float m_temperature{};
      int m_star_id{};
      int m_planet_id{};
      int m_primary_planet_id{};
      body_type m_body_type = body_type::unset;
      std::vector<planet_biome> m_biome_refs;

      explicit planet(const uint32_t formid)
         : m_formid(formid)
         , m_list_item_detector("PPBD - Biome #")
      {

      }


      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);
         extract(line.m_line_content, "TEMP - Temperature in C", m_temperature);
         extract(line.m_line_content, "Star ID", m_star_id);
         extract(line.m_line_content, "Planet ID", m_planet_id);
         extract(line.m_line_content, "Primary planet ID", m_primary_planet_id);

         const auto biome_generator = [](const std::vector<std::string>& lines) -> std::optional<planet_biome> {
            planet_biome biome;
            for (const auto& l : lines)
            {
               extract(l, "Percentage", biome.m_percentage);
               extract(l, "Biome reference", biome.m_biome_ref);
               extract(l, "Resource gen override", biome.m_resource_gen_override);
            }
            return biome;
         };
         m_list_item_detector.process_line(line, m_biome_refs, biome_generator);

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
      uint32_t m_formid;
      std::optional<std::string> m_name;
      std::vector<property> m_properties;
      list_item_detector m_property_builder;

      explicit omod(const uint32_t formid)
         : m_formid(formid)
         , m_property_builder("Property #")
      {
      }


      [[nodiscard]] auto reject() const -> bool
      {
         return false;
      }

      static auto build_property(const std::vector<std::string>& lines) -> std::optional<property>
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
            if(line.starts_with("Value ") && std::regex_match(line, match, pattern))
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
}


auto main() -> int
{
   using namespace pp;
   const auto t0 = std::chrono::steady_clock::now();

   std::map<uint32_t, pp::lctn> star_locations;
   std::map<uint32_t, pp::omod> omods;
   std::map<uint32_t, pp::star> stars;
   std::map<uint32_t, pp::planet> planets;
   std::vector<std::jthread> threads;
   threads.reserve(10);
   threads.emplace_back(pp::run<pp::lctn>, "../data/xdump_lctn.txt", "LCTN", std::ref(star_locations));
   threads.emplace_back(pp::run<pp::omod>, "../data/xdump_omod.txt", "OMOD", std::ref(omods));
   threads.emplace_back(pp::run<pp::star>, "../data/xdump_stars.txt", "STDT", std::ref(stars));
   threads.emplace_back(pp::run<pp::planet>, "../data/xdump_planets.txt", "PNDT", std::ref(planets));
   for (auto& thread : threads)
      thread.join();
   threads.clear();

   const auto t1 = std::chrono::steady_clock::now();
   const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
   std::cout << "ms: " << dt << "\n";

   int end = 0;
}

