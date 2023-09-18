#include "tools.h"

#include <cstdint>
#include <format>
#include <string>


auto pp::from_little(std::string_view str) -> uint32_t
{
   if (str.starts_with("0x"))
   {
      str = str.substr(2);
   }
   uint32_t result{};
   for (int i = 0; i < 4; ++i)
   {
      const int part = std::stoi(std::string(str.substr(2 * i, 2)), nullptr, 16);
      const auto shift_amount = i * 8;
      result |= part << shift_amount;
   }

   return result;
}


auto pp::from_big(std::string_view str) -> uint32_t
{
   if (str.starts_with("0x"))
   {
      str = str.substr(2);
   }
   uint32_t result{};
   for (int i = 0; i < 4; ++i)
   {
      const int part = std::stoi(std::string(str.substr(2 * i, 2)), nullptr, 16);
      const auto shift_amount = (3 - i) * 8;
      result |= part << shift_amount;
   }

   return result;
}


auto pp::as_little(const uint32_t data) -> std::string
{
   std::string result = "0x";
   result.reserve(2 + 4 * 2);
   for (int i = 0; i < 4; ++i)
   {
      const uint32_t shift_amount = i * 8;
      const uint32_t value = 0xff & (data >> shift_amount);
      result += std::format("{:0>2x}", value);
   }
   return result;
}


auto pp::as_big(const uint32_t data) -> std::string
{
   std::string result = "0x";
   result.reserve(2 + 4 * 2);
   for (int i = 0; i < 4; ++i)
   {
      const uint32_t shift_amount = (3 - i) * 8;
      const uint32_t value = 0xff & (data >> shift_amount);
      result += std::format("{:0>2x}", value);
   }
   return result;
}
