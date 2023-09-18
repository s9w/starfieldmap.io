#pragma once

#include <string_view>
#include <string>

#include "fundamentals.h"



namespace pp
{
   auto get_indentation_level(const std::string& line) -> int;

   struct line_content {
      int m_level;
      std::string m_line_content;
      int m_line_number;
   };

   auto get_line_content(std::string&& line, const int line_number) -> line_content;


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
      else
      {
         std::terminate();
      }
      return true;
   }


   auto get_formid(const std::string& line) -> uint32_t;

   auto from_little(std::string_view str) -> uint32_t;
   auto from_big(std::string_view str) -> uint32_t;
   auto as_little(const uint32_t data) -> std::string;
   auto as_big(const uint32_t data) -> std::string;
}


