#pragma once

#include <unordered_map>

#include <cppp/tools.h>


namespace pp {

   template<typename T>
   using formid_map = std::unordered_map<formid, T>;


   template<typename T>
   concept fillable = requires(T t, const line_content & lc)
   {
      {t.process_line(lc)} -> std::same_as<void>;
   };


   template<fillable T>
   auto parse_xedit_output(const std::string_view& filename, const std::string_view name, formid_map<T>& map) -> void
   {
      const file_chopper chopper(filename);
      std::optional<T> next_obj;
      bool looking_for_first_formid = true;
      const std::string name_starter = std::format("FormID: {}", name);
      for (const line_content& content : chopper.m_lines)
      {

         if (content.m_line_content.starts_with(name_starter))
         {
            if (looking_for_first_formid == false)
            {
               if (next_obj.value().reject() == false)
                  map.emplace(next_obj.value().m_formid, next_obj.value());
            }
            else
            {
               looking_for_first_formid = false;
            }
            next_obj.emplace(get_formid(content.m_line_content));
         }
         else if (looking_for_first_formid == false)
         {
            next_obj.value().process_line(content);
         }
      }
      if (next_obj.value().reject() == false)
         map.emplace(next_obj.value().m_formid, next_obj.value());
   }


   enum class prop_function_type { not_set, mul_add, add, set, rem };

   struct property {
      prop_function_type m_function_type = prop_function_type::not_set;
      std::string m_property_name;
      float m_mult;
      float m_add;
      float m_step;

      friend bool operator==(property const& lhs, property const& rhs) = default;
   };


   struct lctn {
      formid m_formid;
      int m_system_level{};
      std::string m_name;
      bool m_reject = true;

      explicit lctn(const formid formid);
      auto process_line(const line_content& line) -> void;
      [[nodiscard]] auto reject() const -> bool;
   };


   struct biom {
      formid m_formid;
      std::string m_name;

      explicit biom(const formid formid);
      auto process_line(const line_content& line) -> void;
      [[nodiscard]] auto reject() const -> bool;
   };


   struct flor {
      formid m_formid;
      std::string m_name;

      explicit flor(const formid formid);
      auto process_line(const line_content& line) -> void;
      [[nodiscard]] auto reject() const -> bool;
   };


   struct stdt {
      formid m_formid;
      std::string m_name;
      float m_x{};
      float m_y{};
      float m_z{};
      bool m_reject = true;
      int m_star_id{};

      explicit stdt(const formid formid);
      auto process_line(const line_content& line) -> void;
      [[nodiscard]] auto reject() const -> bool;
   };


   struct planet_biome_ref {
      int m_percentage{};
      formid m_biome_ref{};
      formid m_resource_gen_override{};
      constexpr friend auto operator==(const planet_biome_ref& a, const planet_biome_ref& b) -> bool
      {
         return a.m_biome_ref == b.m_biome_ref;
      }
   };
   

   enum class body_type { unset, planet, moon };
   struct pndt {
      formid m_formid{};
      bool m_reject = true;
      list_item_detector m_biome_detector;
      list_item_detector m_animal_detector;
      list_item_detector m_flora_detector;
      list_item_detector m_atmosphere_detector;
      list_item_detector m_trait_detector;
      list_item_detector m_temp_detector;

      std::string m_name;
      float m_gravity{};
      int m_temperature{};
      int m_temp_level{};
      int m_star_id{};
      int m_planet_id{};
      int m_primary_planet_id{};
      body_type m_body_type = body_type::unset;
      std::vector<planet_biome_ref> m_biome_refs;
      std::vector<formid> m_animals{};
      std::vector<formid> m_plants;
      int m_oxygen_amount{};
      std::vector<std::string> m_traits;

      explicit pndt(const formid formid);
      auto process_line(const line_content& line) -> void;
      [[nodiscard]] auto reject() const -> bool;
   };


   struct omod {
      formid m_formid;
      std::optional<std::string> m_name;
      std::vector<property> m_properties;
      list_item_detector m_property_builder;

      explicit omod(const formid formid);
      [[nodiscard]] auto reject() const -> bool;
      static auto build_property(const std::vector<std::string_view>& lines, std::vector<property>& target) -> void;
      auto process_line(const line_content& line) -> void;
   };

}
