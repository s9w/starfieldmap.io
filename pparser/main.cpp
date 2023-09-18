#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <map>
#include <variant>
#include <vector>
#include <regex>

#include "fundamentals.h"


namespace pp
{

   auto get_indentation_level(const std::string& line) -> int
   {
      int i = 0;
      while (line[i] == ' ')
         ++i;
      return i/2;
   }

   struct line_content{
      int m_level;
      std::string m_line_content;
      int m_line_number;
   };

   auto get_line_content(std::string&& line, const int line_number) -> line_content
   {
      const auto level = get_indentation_level(line);
      std::string content = std::move(line);
      content.erase(0, level * 2);
      return { level, std::move(content), line_number };
   }

   template<typename T>
   auto extract(
      const std::string& line,
      const std::string_view start,
      T& target
   ) -> bool
   {
      if (line.starts_with(start) == false)
         return false;
      if constexpr(is_optional<T>)
      {
         target.emplace();
      }

      const auto value_substr = line.substr(start.size() + 2);
      if constexpr(is_T_or_opt_T<T, std::string>)
      {
         target = value_substr;
      }
      else if constexpr (is_T_or_opt_T<T, float>)
      {
         target = std::stof(value_substr);
      }
      else
      {
         std::terminate();
      }
      return true;
   }

   

   auto get_formid(const std::string& line) -> std::string
   {
      const auto start = line.find('[')+1;
      const auto end = line.find(']');
      return line.substr(start, end - start);
   }

   template<typename T>
   concept fillable = requires(T t, const line_content& lc)
   {
      {t.process_line(lc)} -> std::same_as<void>;
   };


   template<fillable T>
   auto run(const std::string& filename) -> void
   {
      std::ifstream f(filename);
      std::string line;
      int line_number = 1;
      std::map<std::string, T> map;
      std::optional<T> next_obj;
      bool looking_for_first_formid = true;
      while (std::getline(f, line))
      {
         const line_content content = get_line_content(std::move(line), line_number);

         if (content.m_line_content.starts_with("FormID: OMOD"))
         {
            if(looking_for_first_formid == false)
            {
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
      map.emplace(next_obj.value().m_formid, next_obj.value());

      int end = 0;
   }

   enum class prop_function_type{not_set, mul_add, add, set, rem};

   struct property {
      prop_function_type m_function_type = prop_function_type::not_set;
      std::string m_property_name;
      float m_mult;
      float m_add;
      float m_step;
   };

   struct obj {
      std::string m_formid;
      std::optional<std::string> m_name;
      std::vector<property> m_properties;

      std::optional<int> m_in_property_level;
      std::vector<std::string> m_next_property_strings;

      explicit obj(const std::string& formid)
         : m_formid(formid)
      {
         m_next_property_strings.reserve(6);
      }
      auto build_property() -> void
      {
         const std::regex pattern("Value (\\d) - (.+?): (.+)");
         std::string property_name;
         float value1{};
         float value2{};
         prop_function_type fun_type = prop_function_type::not_set;
         float step{};
         for(const auto& line : m_next_property_strings)
         {
            std::string fun_type_str;
            extract(line, "Function Type", fun_type_str);
            if(fun_type_str.empty() == false)
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
                  m_next_property_strings.clear();
                  return;
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
         m_next_property_strings.clear();
         m_properties.emplace_back(
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

         // start property
         if (line.m_line_content.starts_with("Property #"))
         {
            if(m_next_property_strings.empty() == false)
            {
               this->build_property();
            }
            m_in_property_level.emplace(line.m_level);
         }

         // prop is active
         else if (m_in_property_level.has_value())
         {
            if(line.m_level <= m_in_property_level.value())
            {
               // end of this property because property list ends
               this->build_property();
               m_in_property_level.reset();
            }
            m_next_property_strings.push_back(line.m_line_content);
         }
         
            
      }
   };
}


auto main() -> int
{
   pp::run<pp::obj>("../data/xdump_omod.txt");
}

