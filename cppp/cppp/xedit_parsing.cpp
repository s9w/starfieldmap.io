#include "xedit_parsing.h"

#include <boost/regex.hpp>


pp::lctn::lctn(const formid formid)
   : m_formid(formid)
{

}


auto pp::lctn::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "System level", m_system_level);
   extract(line.m_line_content, "FULL - Name", m_name);

   std::string m_parent_location{};
   if (extract(line.m_line_content, "PNAM - Parent Location", m_parent_location) && get_formid(m_parent_location) == static_cast<formid>(0x0001a53a))
   {
      m_reject = false;
   }
}


auto pp::lctn::reject() const -> bool
{
   return m_reject;
}


pp::biom::biom(const formid formid)
: m_formid(formid) {

}


auto pp::biom::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "FULL - Name", m_name);
}


auto pp::biom::reject() const -> bool
{
   return false;
}



pp::flor::flor(const formid formid)
: m_formid(formid)
{

}


auto pp::flor::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "FULL - Name", m_name);
}


auto pp::flor::reject() const -> bool
{
   return false;
}



pp::stdt::stdt(const formid formid)
: m_formid(formid)
{
}


auto pp::stdt::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "FULL - Name", m_name);
   extract(line.m_line_content, "DNAM - Star ID", m_star_id);
   extract(line.m_line_content, "x", m_x);
   extract(line.m_line_content, "y", m_y);
   extract(line.m_line_content, "z", m_z);

   if (m_name.empty() == false)
      m_reject = false;
}


auto pp::stdt::reject() const -> bool
{
   return m_reject;
}



pp::pndt::pndt(const formid formid): m_formid(formid)
                                     , m_biome_detector("PPBD - Biome #")
                                     , m_animal_detector("Fauna #")
                                     , m_flora_detector("Flora #")
                                     , m_atmosphere_detector("Keyword #")
                                     , m_trait_detector("Keyword #")
                                     , m_temp_detector("Keyword #") {

}


auto pp::pndt::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "FULL - Name", m_name);
   extract(line.m_line_content, "TEMP - Temperature in C", m_temperature);
   extract(line.m_line_content, "Star ID", m_star_id);
   extract(line.m_line_content, "Planet ID", m_planet_id);
   extract(line.m_line_content, "Primary planet ID", m_primary_planet_id);
   extract(line.m_line_content, "Gravity", m_gravity);

   const auto biome_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      planet_biome_ref biome_ref;
      for (const auto& l : lines)
      {
         extract(l, "Percentage", biome_ref.m_percentage);
         extract(l, "Biome reference", biome_ref.m_biome_ref);
         extract(l, "Resource gen override", biome_ref.m_resource_gen_override);
      }
      if (std::ranges::find(target, biome_ref) == std::cend(target))
         target.emplace_back(biome_ref);
   };
   m_biome_detector.process_line(line, m_biome_refs, biome_generator);

   const auto animal_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      pp_assert(lines.size() == 2);
      const auto result = get_formid(lines[1]);
      if (std::ranges::find(target, result) == std::cend(target))
         target.emplace_back(result);
   };
   m_animal_detector.process_line(line, m_animals, animal_generator);

   const auto flora_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      std::string buffer;
      for (const auto& l : lines)
      {
         if (extract(l, "Model", buffer))
         {
            const auto result = get_formid(l);
            if (std::ranges::find(target, result) == std::cend(target))
               target.emplace_back(result);
         }
      }
   };
   m_flora_detector.process_line(line, m_plants, flora_generator);

   const auto keyword_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      if (lines.size() == 1)
      {
         if (lines[0].find("PlanetAtmosphereType07LowO2") != std::string::npos)
            target = 18;
         else if (lines[0].find("PlanetAtmosphereType05O2") != std::string::npos)
            target = 21;
         else if (lines[0].find("PlanetAtmosphereType06HighO2") != std::string::npos)
            target = 24;
      }
   };
   m_atmosphere_detector.process_line(line, m_oxygen_amount, keyword_generator);

   const auto trait_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      if (lines.size() == 1)
      {
         const auto name = get_between(lines[0], '<', '>');
         if (name.starts_with("PlanetTrait"))
         {
            std::string result(get_between(lines[0], '\"', '\"'));
            if (std::ranges::find(target, result) == std::cend(target))
               target.emplace_back(result);
         }
      }
   };
   m_trait_detector.process_line(line, m_traits, trait_generator);

   const auto temp_generator = [](const std::vector<std::string_view>& lines, auto& target) -> void {
      if (lines.size() == 1)
      {
         const auto name = get_between(lines[0], '<', '>');
         if (name.starts_with("PlanetTemperature"))
         {
            std::from_chars(name.data() + 18, name.data() + 19, target);
         }
      }
   };
   m_temp_detector.process_line(line, m_temp_level, temp_generator);

   std::string body_type_str;
   if (extract(line.m_line_content, "CNAM - Body type", body_type_str))
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


auto pp::pndt::reject() const -> bool
{
   return m_reject;
}


pp::omod::omod(const formid formid)
   : m_formid(formid)
   , m_property_builder("Property #")
{

}


auto pp::omod::reject() const -> bool
{
   return false;
}


auto pp::omod::build_property(const std::vector<std::string_view>& lines, std::vector<property>& target) -> void
{
   const boost::regex pattern("Value (\\d) - (.+?): (.+)");
   std::string property_name;
   float value1{};
   float value2{};
   prop_function_type fun_type = prop_function_type::not_set;
   float step{};
   for (const auto& line : lines)
   {
      std::string fun_type_str;
      if (extract(line, "Function Type", fun_type_str))
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
      if (line.starts_with("Value ") && boost::regex_match(line_str, match, pattern)) // TODO: wow this is atrocious
      {
         if (match[2] == "FormID")
         {
            return;
         }
         float value{};
         if (match[2] == "Float")
            value = std::stof(match[3]);
         else if (match[2] == "Int")
            value = static_cast<float>(std::stoi(match[3]));

         if (match[1] == "1")
            value1 = value;
         else if (match[1] == "2")
            value2 = value;
      }
   }
   if (fun_type == prop_function_type::not_set)
      std::terminate();
   const property result {
      .m_function_type = fun_type,
      .m_property_name = property_name,
      .m_mult = value1,
      .m_add = value2,
      .m_step = step
   };
   if (std::ranges::find(target, result) == std::cend(target))
      target.emplace_back(result);
}


auto pp::omod::process_line(const line_content& line) -> void
{
   extract(line.m_line_content, "FULL - Name", m_name);

   m_property_builder.process_line(line, m_properties, omod::build_property);
}
