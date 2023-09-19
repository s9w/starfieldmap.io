#include "tools.h"

#include <cstdint>
#include <format>
#include <string>
#include <iostream>
#include <chrono>


pp::ms_timer::ms_timer()
   : m_t0(std::chrono::steady_clock::now())
{
   
}


pp::ms_timer::~ms_timer()
{
   const auto t1 = std::chrono::steady_clock::now();
   const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - m_t0);
   std::cout << "ms: " << dt << "\n";
}


auto pp::get_indentation_level(const std::string_view& line) -> int
{
   int i = 0;
   while (line[i] == ' ')
      ++i;
   return i / 2;
}


auto pp::get_formid(const std::string_view line) -> formid
{
   return from_big(get_between(line, '[', ']'));
}


constexpr auto pp::get_between(const std::string_view line, const char left, const char right) -> std::string_view
{
   const auto start = line.find(left) + 1;
   const auto end = line.rfind(right);
   return line.substr(start, end - start);
}
static_assert(pp::get_between("a_bc_d", '_', '_') == "bc");


auto pp::from_little(std::string_view str) -> formid
{
   if (str.starts_with("0x"))
   {
      str = str.substr(2);
   }
   formid result{};
   for (int i = 0; i < 4; ++i)
   {
      const formid part = static_cast<formid>(std::stoi(std::string(str.substr(2 * i, 2)), nullptr, 16));
      const auto shift_amount = i * 8;
      result |= part << shift_amount;
   }

   return result;
}


auto pp::from_big(std::string_view str) -> formid
{
   if (str.starts_with("0x"))
   {
      str = str.substr(2);
   }
   formid result{};
   for (int i = 0; i < 4; ++i)
   {
      const formid part = static_cast<formid>(std::stoi(std::string(str.substr(2 * i, 2)), nullptr, 16));
      const auto shift_amount = (3 - i) * 8;
      result |= part << shift_amount;
   }

   return result;
}


auto pp::as_little(const formid data) -> std::string
{
   std::string result = "";
   result.reserve(2 + 4 * 2);
   for (int i = 0; i < 4; ++i)
   {
      const auto shift_amount = i * 8;
      const formid value = static_cast<formid>(0xff) & (data >> shift_amount);
      result += std::format("{:0>2x}", value.value_of());
   }
   return result;
}


auto pp::as_big(const formid data) -> std::string
{
   std::string result = "";
   result.reserve(2 + 4 * 2);
   for (int i = 0; i < 4; ++i)
   {
      const auto shift_amount = (3 - i) * 8;
      const formid value = static_cast<formid>(0xff) & (data >> shift_amount);
      result += std::format("{:0>2x}", value.value_of());
   }
   return result;
}
