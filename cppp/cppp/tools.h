#pragma once

#include <string_view>
#include <string>
#include <vector>

#include <cppp/fundamentals.h>

#include <cppp/strong_type/strong_type.hpp>


namespace pp
{
   using formid = strong::type<uint32_t, struct my_formid, strong::regular, strong::bitarithmetic, strong::ordered >;
   auto get_indentation_level(const std::string& line) -> int;

   struct line_content {
      int m_level;
      std::string m_line_content;
      int m_line_number;
   };

   auto get_line_content(std::string&& line, const int line_number) -> line_content;
   auto get_formid(const std::string& line) -> formid;

   template<typename T>
   auto extract(
      const std::string& line,
      const std::string_view start,
      T& target
   ) -> bool
   {
      if (line.starts_with(start) == false)
         return false;
      if constexpr (is_optional<T>)
      {
         target.emplace();
      }

      const auto value_substr = line.substr(start.size() + 2);
      if constexpr (is_T_or_opt_T<T, std::string>)
      {
         target = value_substr;
      }
      else if constexpr (is_T_or_opt_T<T, float>)
      {
         target = std::stof(value_substr);
      }
      else if constexpr (is_T_or_opt_T<T, int>)
      {
         target = std::stoi(value_substr);
      }
      else if constexpr (is_T_or_opt_T<T, formid>)
      {
         target = get_formid(value_substr);
      }
      else
      {
         std::terminate();
      }
      return true;
   }

   struct list_item_detector {
      std::optional<int> m_in_property_level;
      std::vector<std::string> m_next_property_strings;
      std::string m_start_str;

      explicit list_item_detector(const std::string start_str)
         : m_start_str(start_str)
      {
         m_next_property_strings.reserve(10);
      }
      template<typename T>
      auto process_line(const line_content& line, std::vector<T>& target, const auto& m_lambda) -> void
      {
         if (line.m_line_content.starts_with(m_start_str))
         {
            if (m_next_property_strings.empty() == false)
            {
               const auto result = m_lambda(m_next_property_strings);
               if(result.has_value())
                  target.emplace_back(*result);

               m_next_property_strings.clear();
            }
            m_in_property_level.emplace(line.m_level);
         }

         // prop is active
         else if (m_in_property_level.has_value())
         {
            if (line.m_level <= m_in_property_level.value())
            {
               // end of this property because property list ends
               const auto result = m_lambda(m_next_property_strings);
               if (result.has_value())
                  target.emplace_back(*result);

               m_in_property_level.reset();
               m_next_property_strings.clear();
            }
            m_next_property_strings.push_back(line.m_line_content);
         }
      }
   };
   

   auto from_little(std::string_view str) -> formid;
   auto from_big(std::string_view str) -> formid;
   auto as_little(const formid data) -> std::string;
   auto as_big(const formid data) -> std::string;
}


