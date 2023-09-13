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

   enum class global_mode{starting, waiting_for_record_header, waiting_for_formid};

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
      if constexpr(std::same_as<T, std::string> || (is_optional<T> && std::same_as<typename T::value_type, std::string>))
      {
         target = line.m_line_content.substr(start.size() + 2);
      }
      else if constexpr (std::same_as<T, float>)
      {
         try
         {
            target = std::stof(line.m_line_content.substr(0, start.size() + 2));
         }
         catch(...)
         {
            std::terminate();
         }
      }
   }

   struct obj{
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

   auto get_formid(const std::string& line) -> std::string
   {
      const auto start = line.find('[')+1;
      const auto end = line.find(']');
      return line.substr(start, end - start);
   }


   auto run() -> void
   {
      global_mode mode = global_mode::starting;
      std::ifstream f("../data/xdump_omod.txt");
      std::string line;
      int line_number = 1;
      std::map<std::string, obj> map;
      std::optional<obj> next_obj;
      while (std::getline(f, line))
      {
         const line_content content = get_line_content(line);

         if (content.m_level > 0 && content.m_line_content == "Record Header")
         {
            if (mode == global_mode::waiting_for_record_header)
            {
               pp_assert(next_obj.has_value());
               map.emplace(next_obj.value().m_formid, next_obj.value());
               next_obj.reset();
            }
            else if (mode == global_mode::starting)
            {
               // Don't commit empty object
            }
            else
            {
               std::terminate();
            }
            mode = global_mode::waiting_for_formid;
         }
         else if (mode == global_mode::waiting_for_formid && content.m_level > 0 && content.m_line_content.starts_with("FormID: "))
         {
            pp_assert(next_obj.has_value() == false);
            next_obj.emplace(get_formid(content.m_line_content));
            mode = global_mode::waiting_for_record_header;
         }
         else if(mode == global_mode::waiting_for_record_header)
         {
            pp_assert(next_obj.has_value());
            next_obj.value().process_line(content);
         }
         ++line_number;
      }
      map.emplace(next_obj.value().m_formid, next_obj.value());


      int end = 0;
   }
}


auto main() -> int
{
   pp::run();
}
