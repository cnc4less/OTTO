#pragma once

#include "type_traits.hpp"

namespace otto::core::props {

  /// Metafunction that adds all of the required tags for all tags in
  /// `TagList` to the list.
  template<typename TagList>
  struct add_required;

  template<typename... Tags>
  struct add_required<tag_list<Tags...>> {
    using type = meta::flatten_t<
      meta::concat_t<tag_list<Tags, MixinTag::required_tags_t<Tags>>...>>;
  };

  constexpr bool compare_str(const char* a, const char* b)
  {
    for (; (*a != '\0') && (*b != '\0'); ++a, (void) ++b) {
      if (*a < *b) return true;
      if (*b < *a) return false;
    }
    return (*a == '\0') && (*b != '\0');
  }

  template<typename Tag1, typename Tag2>
  struct compare_tags {
    constexpr static bool value = compare_str(Tag1::name, Tag2::name);
  };

  /// Uniquify and sort tags by their name
  template<typename TagList>
  struct normalize_tags {
    using type = meta::sort_t<meta::uniquify_t<TagList>, compare_tags>;
  };

  /// Simple type that inherits publicly from all of `SuperClasses...`
  ///
  /// Also uses all operators and constructors
  template<typename... SuperClasses>
  struct inherit_from_all : SuperClasses... {
    using SuperClasses::operator=...;
  };

  /// Type trait to see if a tag_list contains a tag.
  template<typename TagList, typename T>
  constexpr const bool contains_tag_v = meta::_v<meta::contains<TagList, T>>;

  /// Metafunction to get a type that inherits from all the mixins
  /// coresponding to TagList.
  ///
  /// Does not add required mixins.
  ///
  /// \requires `TagList == tag_list<...>`
  /// \returns a `boost::hana::type` containing the result type
  template<typename ValueType, typename TagList>
  struct inherit_from_all_mixins;

  template<typename ValueType, typename... Tags>
  struct inherit_from_all_mixins<ValueType, tag_list<Tags...>> {
    using type = inherit_from_all<
      MixinTag::mixin_t<Tags, ValueType, tag_list<Tags...>>...>;
  };

  /// This type inherits from all mixins matched by `Tags`
  ///
  /// \see inherit_from_all_mixins_impl
  /// \requires `TagList == tag_list<...>`
  template<typename ValueType, typename TagList>
  using inherits_from_mixins_t = meta::_t<inherit_from_all_mixins<ValueType, TagList>>;

  /// Make a tag_list type with all `Tags` and their requirements
  template<typename... Tags>
  using make_tag_list_t =
    meta::_t<normalize_tags<meta::_t<add_required<tag_list<Tags...>>>>>;


  template<typename Tag, typename... Args>
  struct TaggedTuple {
    using tag_type = Tag;
    std::tuple<Args...> args;

    static constexpr bool _this_is_a_mixin_initializer_type_ = true;
  };

  template<typename Tag, typename... Args>
  auto make_initializer(Args&&... args)
  {
    return TaggedTuple<Tag, Args...>{
      std::forward_as_tuple(std::forward<Args>(args)...)};
  }

  template<typename Initializer, typename = void>
  struct is_initializer : std::false_type {};

  template<typename Initializer>
  struct is_initializer<
    Initializer,
    std::void_t<decltype(Initializer::_this_is_a_mixin_initializer_type_),
                typename Initializer::tag_type>> {
    constexpr static const bool value =
      Initializer::_this_is_a_mixin_initializer_type_;
  };

  template<typename Initializer>
  constexpr static bool is_initializer_v = is_initializer<Initializer>::value;

} // namespace otto::core::props
