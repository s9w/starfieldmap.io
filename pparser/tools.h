#pragma once

#include <string_view>

namespace pp
{
   auto from_little(std::string_view str) -> uint32_t;
   auto from_big(std::string_view str) -> uint32_t;
   auto as_little(const uint32_t data) -> std::string;
   auto as_big(const uint32_t data) -> std::string;
}


