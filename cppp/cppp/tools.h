#pragma once

#include <string_view>
#include <string>
#include <vector>

#include <cppp/fundamentals.h>

#include <strong_type/strong_type.hpp>


namespace pp
{
   using formid = strong::type<uint32_t, struct my_formid, strong::regular, strong::bitarithmetic, strong::ordered >;
}

template <>
struct std::hash<pp::formid>
{
   constexpr auto operator()(const pp::formid& k) const noexcept -> std::size_t
   {
      return k.value_of();
   }
};

namespace pp{
   struct ms_timer{
      std::chrono::steady_clock::time_point m_t0;
      explicit ms_timer();
      ~ms_timer();

      ms_timer(const ms_timer&) = delete;
      ms_timer& operator=(const ms_timer&) = delete;
      ms_timer(ms_timer&&) = delete;
      ms_timer& operator=(ms_timer&&) = delete;
   };

   auto get_indentation_level(const std::string_view& line) -> int;
   auto get_lower(const std::string_view in) -> std::string;

   struct line_content {
      int m_level;
      std::string_view m_line_content;
      int m_line_number;
   };

   auto get_formid(const std::string_view line) -> formid;
   constexpr auto get_between(const std::string_view line, const char left, const char right) -> std::string_view;

   auto compress(const std::vector<std::byte>& src) -> std::vector<std::byte>;
   auto compress(const std::string& src) -> std::vector<std::byte>;
   auto write_binary_file(
      const std::string_view filename,
      const std::vector<std::byte>& vec
   ) -> void;

   struct biome {
      int m_percentage{};
      std::string m_name;
      formid m_formid;
   };

   struct flora {
      std::string m_name;
      formid m_formid;
   };

   struct body {
      formid m_formid;
      std::string m_name;
      int m_temperature{};
      int m_temp_level;
      std::string m_gravity{};
      int m_fauna_count{};
      std::vector<flora> m_flora;
      int m_oxygen_amount{};
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

   template<typename T>
   auto extract(
      const std::string_view& line,
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
         std::from_chars(value_substr.data(), value_substr.data() + value_substr.size(), target);
      }
      else if constexpr (is_T_or_opt_T<T, int>)
      {
         std::from_chars(value_substr.data(), value_substr.data()+ value_substr.size(), target);
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

   struct animal;
   struct list_item_detector {
      std::optional<int> m_in_property_level;
      std::vector<std::string_view> m_next_property_strings;
      std::string m_start_str;

      explicit list_item_detector(const std::string_view start_str)
         : m_start_str(start_str)
      {
         m_next_property_strings.reserve(10);
      }
      template<typename T>
      auto process_line(const line_content& line, T& target, const auto& m_lambda) -> void
      {
         if (line.m_line_content.starts_with(m_start_str))
         {
            if (m_next_property_strings.empty() == false)
            {
               m_lambda(m_next_property_strings, target);

               m_next_property_strings.clear();
            }
            m_in_property_level.emplace(line.m_level);
            m_next_property_strings.push_back(line.m_line_content);
         }

         // prop is active
         else if (m_in_property_level.has_value())
         {
            if (line.m_level <= m_in_property_level.value())
            {
               // end of this property because property list ends
               m_lambda(m_next_property_strings, target);

               m_in_property_level.reset();
               m_next_property_strings.clear();
            }
            else
            {
               m_next_property_strings.push_back(line.m_line_content);
            }
         }
      }
   };
   

   auto from_little(std::string_view str) -> formid;
   auto from_big(std::string_view str) -> formid;
   auto as_little(const formid data) -> std::string;
   auto as_big(const formid data) -> std::string;
}


