#pragma once

#include <exception>
#include <string_view>


namespace pp
{
   constexpr auto pp_assert(const bool condition, const std::string_view msg) -> void;
   constexpr auto pp_assert(const bool condition) -> void;


   template<typename>   constexpr bool is_optional_impl = false;
   template<typename T> constexpr bool is_optional_impl<std::optional<T>> = true;

   template<typename T>
   constexpr bool is_optional = is_optional_impl<std::remove_cvref_t<T>>;
}

constexpr auto pp::pp_assert(const bool condition, const std::string_view msg) -> void
{
   if (condition == false)
   {
      // log::error(msg);
      std::terminate();
   }
}


constexpr auto pp::pp_assert(const bool condition) -> void
{
   if (condition == false)
   {
      // log::error("error without msg");
      std::terminate();
   }
}