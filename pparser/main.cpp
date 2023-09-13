#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <map>

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
   };

   auto get_line_content(const std::string& line) -> line_content
   {
      const auto level = get_indentation_level(line);
      std::string content = line;
      content.erase(0, level * 2);
      return { level, content };
   }

   template<typename T>
   auto extract(
      const line_content& line,
      const std::string& start,
      T& target
   ) -> void
   {
      if (line.m_line_content.starts_with(start) == false)
         return;
      if constexpr(is_optional<T>)
      {
         target.emplace();
      }

      const auto value_substr = line.m_line_content.substr(start.size() + 2);
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
      // global_mode mode = global_mode::starting;
      std::ifstream f(filename);
      std::string line;
      int line_number = 1;
      std::map<std::string, T> map;
      std::optional<T> next_obj;
      bool looking_for_first_formid = true;
      while (std::getline(f, line))
      {
         const line_content content = get_line_content(line);

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


   struct obj {
      std::string m_formid;
      std::optional<std::string> m_name;
      explicit obj(const std::string& formid)
         : m_formid(formid)
      {

      }
      auto process_line(const line_content& line) -> void
      {
         extract(line, "FULL - Name", m_name);
      }
   };
}


auto main() -> int
{
   pp::run<pp::obj>("../data/xdump_omod.txt");
}
