#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <map>
#include <variant>
#include <vector>
#include <chrono>
#include <ranges>

#include <cppp/tools.h>
#include <boost/regex.hpp>

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


   struct biom {
      formid m_formid;
      std::string m_name;

      explicit biom(const formid formid)
         : m_formid(formid)
      {

      }
      auto process_line(const line_content& line) -> void
      {
         extract(line.m_line_content, "FULL - Name", m_name);
      }

      [[nodiscard]] auto reject() const -> bool
      {
         return false;
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
   

   struct planet_biome_ref{
      float m_percentage{};
      formid m_biome_ref{};
      formid m_resource_gen_override{};
   };
   auto operator==(const planet_biome_ref& a, const planet_biome_ref& b) -> bool
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
      list_item_detector m_atmosphere_detector;
      list_item_detector m_trait_detector;

      std::string m_name;
      float m_gravity;
      float m_temperature{};
      int m_star_id{};
      int m_planet_id{};
      int m_primary_planet_id{};
      body_type m_body_type = body_type::unset;
      std::vector<planet_biome_ref> m_biome_refs;
      std::vector<animal> m_animal_refs;
      std::vector<plant> m_flora_refs;
      std::vector<float> m_oxygen_amount;
      std::vector<std::string> m_traits;

      explicit pndt(const formid formid)
         : m_formid(formid)
         , m_biome_detector("PPBD - Biome #")
         , m_animal_detector("Fauna #")
         , m_flora_detector("Flora #")
         , m_atmosphere_detector("Keyword #")
         , m_trait_detector("Keyword #")
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

         const auto biome_generator = [](const std::vector<std::string_view>& lines) -> std::optional<planet_biome_ref> {
            planet_biome_ref biome_ref;
            for (const auto& l : lines)
            {
               extract(l, "Percentage", biome_ref.m_percentage);
               extract(l, "Biome reference", biome_ref.m_biome_ref);
               extract(l, "Resource gen override", biome_ref.m_resource_gen_override);
            }
            return biome_ref;
         };
         m_biome_detector.process_line(line, m_biome_refs, biome_generator);

         const auto animal_generator = [](const std::vector<std::string_view>& lines) -> std::optional<animal> {
            pp_assert(lines.size() == 2);
            return animal{.m_formid = get_formid(lines[1])};
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

         if (line.m_line_number == 531)
            int stop = 0;

         const auto keyword_generator = [](const std::vector<std::string_view>& lines) -> std::optional<float> {
            if (lines.size() == 1)
            {
               if (lines[0].find("PlanetAtmosphereType07LowO2") != std::string::npos)
                  return 18.0f;
               if (lines[0].find("PlanetAtmosphereType05O2") != std::string::npos)
                  return 21.0f;
               if (lines[0].find("PlanetAtmosphereType06HighO2") != std::string::npos)
                  return 24.0f;
            }

            return std::nullopt;
            };
         m_atmosphere_detector.process_line(line, m_oxygen_amount, keyword_generator);

         const auto trait_generator = [](const std::vector<std::string_view>& lines) -> std::optional<std::string> {
            if (lines.size() == 1)
            {
               const auto name = get_between(lines[0], '<', '>');
               if(name.starts_with("PlanetTrait"))
                  return std::string(get_between(lines[0], '\"', '\"'));
            }
            return std::nullopt;
            };
         m_trait_detector.process_line(line, m_traits, trait_generator);

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
         const boost::regex pattern("Value (\\d) - (.+?): (.+)");
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


            boost::smatch match;
            std::string line_str(line);
            if(line.starts_with("Value ") && boost::regex_match(line_str, match, pattern)) // TODO: wow this is atrocious
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

   struct biome{
      float m_percentage{};
      std::string m_name;
   };

   struct body {
      formid m_formid;
      std::string m_name;
      float m_temperature{};
      float m_gravity{};
      std::vector<animal> m_fauna;
      std::vector<plant> m_flora;
      float m_oxygen_amount{};
      std::vector<biome> m_biomes;
      std::vector<std::string> m_traits;
   };
   struct planet : body {
      std::vector<body> m_moons;
      int m_planet_id{};
   };

   struct star {
      formid m_formid;
      float m_x{};
      float m_y{};
      float m_z{};
      int m_level{};
      std::string m_name;
      std::vector<planet> m_planets;
   };


   auto get_oxygen_amount(const std::vector<float>& vec) -> float
   {
      if (vec.empty()) return 0.0f;
      pp_assert(vec.size() == 1);
      return vec[0];
   }

   auto get_biomes(const std::vector<planet_biome_ref>& refs, const formid_map<pp::biom>& bioms) -> std::vector<biome>
   {
      std::vector<biome> result;
      result.reserve(refs.size());
      for(const auto& ref : refs)
      {
         result.push_back(
            biome{
               .m_percentage = ref.m_percentage,
               .m_name = bioms.at(ref.m_biome_ref).m_name
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

   auto build_universe(
      const formid_map<pp::lctn>& lctns,
      const formid_map<pp::stdt>& stdts,
      const formid_map<pp::pndt>& pndts,
      const formid_map<pp::biom>& bioms
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
            .m_gravity = value.m_gravity,
            .m_fauna = value.m_animal_refs,
            .m_flora = value.m_flora_refs,
            .m_oxygen_amount = get_oxygen_amount(value.m_oxygen_amount),
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

      return starid_to_star;
   }


   constexpr int indent_size = 4;
   auto print(std::string& str, const body& b, const int indentation) -> void
   {
      const std::string indentation_str = std::string(indentation, ' ');

      std::string biome_str;
      for(const auto& bi : b.m_biomes)
      {
         const bool first = biome_str.empty();
         if (first == false)
            biome_str += ", ";
         biome_str += std::format("{}% {}", bi.m_percentage, bi.m_name);
      }

      str += std::format(
         "{}{} ({}), {}C, {}g, {}% O2, flora: {}, fauna: {}; {} traits; {}\n",
         indentation_str, b.m_name, as_big(b.m_formid), b.m_temperature, b.m_gravity, b.m_oxygen_amount, b.m_flora.size(), b.m_fauna.size(), b.m_traits.size(), biome_str
      );
   }

   auto print(std::string& str, const planet& p, const int indentation) -> void
   {
      const body base = static_cast<body>(p);
      print(str, base, indentation);
      const std::string indentation_str = std::string(indentation, ' ');
      if(p.m_moons.empty() == false)
      {
         for(const auto& m : p.m_moons)
         {
            print(str, m, indentation + indent_size);
         }
      }
   }

   auto print(std::string& str, const star& s, const int indentation) -> void
   {
      str += std::format("{} system (Level {}; {})\n", s.m_name, s.m_level, as_big(s.m_formid));
      for (const auto& p : s.m_planets)
      {
         print(str, p, indentation + indent_size);
      }
   }

   auto print(const std::unordered_map<int, star>& universe)
   {
      std::string str;
      for (const auto& star : universe | std::views::values)
      {
         print(str, star, 0);
      }
      std::ofstream file("out.txt");
      file << str;
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
   formid_map<pp::biom> bioms;
   std::vector<std::jthread> threads;
   threads.reserve(10);
   threads.emplace_back(pp::run<pp::lctn>, "../../data/xdump_lctn.txt", "LCTN", std::ref(lctns));
   threads.emplace_back(pp::run<pp::stdt>, "../../data/xdump_stars.txt", "STDT", std::ref(stdts));
   threads.emplace_back(pp::run<pp::pndt>, "../../data/xdump_planets.txt", "PNDT", std::ref(pndts));
   threads.emplace_back(pp::run<pp::omod>, "../../data/xdump_omod.txt", "OMOD", std::ref(omods));
   threads.emplace_back(pp::run<pp::biom>, "../../data/xdump_biomes.txt", "BIOM", std::ref(bioms));
   threads.clear();
   timer.emplace();

   

   const std::unordered_map<int, star> universe = build_universe(lctns, stdts, pndts, bioms);
   print(universe);


   [[maybe_unused]] const auto& narion = universe.at(88327);
   timer.reset();

   [[maybe_unused]] int end = 0;
}

