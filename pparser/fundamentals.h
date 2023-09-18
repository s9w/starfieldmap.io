#pragma once

#include <exception>
#include <string_view>
#include <optional>


namespace pp
{
   constexpr auto pp_assert(const bool condition, const std::string_view msg) -> void;
   constexpr auto pp_assert(const bool condition) -> void;


   template<typename>   constexpr bool is_optional_impl = false;
   template<typename T> constexpr bool is_optional_impl<std::optional<T>> = true;

   template<typename T>
   constexpr bool is_optional = is_optional_impl<std::remove_cvref_t<T>>;

   template<typename T>
   struct opt_base_type{
      using type = T;
   };
   template<typename T>
   struct opt_base_type<std::optional<T>>{
      using type = T;
   };
   template<typename T>
   using opt_base_type_t = typename opt_base_type<T>::type;
   static_assert(std::same_as<opt_base_type_t<float>, float>);
   static_assert(std::same_as<opt_base_type_t<std::optional<float>>, float>);

   template<typename T, typename target_type>
   constexpr bool is_T_or_opt_T = std::same_as<opt_base_type_t<T>, target_type>;
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