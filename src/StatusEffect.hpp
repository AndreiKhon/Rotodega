
#ifndef STATUSEFFECT
#define STATUSEFFECT

#include "StatusEffects.hpp"
#include "godot_cpp/classes/node.hpp"
#include <algorithm>
#include <concepts>
#include <format>
#include <source_location>
#include <variant>

namespace game {

template <typename T>
concept EffectConcept = requires(T effect) {
  { effect.value } -> std::convertible_to<std::int64_t>;
  { effect.deplete } -> std::convertible_to<std::int64_t>;
};

template <typename T>
concept MaxEffectConcept =
    EffectConcept<T> && requires(T effect) { // TODO rename
      { effect.max } -> std::convertible_to<std::int64_t>;
    };

using EffectType = std::variant<Slow, Haste>;
using EffectVector = std::vector<EffectType>;

// Check if all EffectType alternatives have 'value' at compile time
// -------------------------
template <std::size_t... i> inline void EffectTest(std::index_sequence<i...>) {
  static_assert((EffectConcept<decltype(std::get<i>(EffectType{}))> && ...));
}

inline void EffectsTest() {
  constexpr auto size = std::variant_size_v<EffectType>;
  EffectTest(std::make_index_sequence<size>{});
}
// ------------------------------------------------------------------------------------------------

class StatusEffect : public godot::Node {
  GDCLASS(StatusEffect, godot::Node)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;

private:
  auto on_timer() -> void;

public:
  auto Update(EffectType effect) -> void;
  auto SetType(EffectType effect) -> void;
  auto GetType() -> EffectType const;

private:
  EffectType effect;
};

// Visitors

struct EffectValueVisitor {
  auto operator()(EffectConcept auto &&effect) { return effect.value; }
};

struct IsSameEffectVisitor {
  auto operator()(auto lhs, auto rhs) -> bool {
    return std::is_same_v<decltype(lhs), decltype(rhs)>;
  }
};

struct DepleteEffectVisitor {
  auto operator()(EffectConcept auto effect) -> EffectType {
    return decltype(effect){effect.value - effect.deplete};
  }
};

struct UpdateEffectVisitor { // Is it possible to remove duplication?

  template <typename T>
    requires MaxEffectConcept<T>
  auto operator()(T lhs, T rhs) -> EffectType {
    auto value = lhs.value + rhs.value;
    value = std::clamp(value, value, T::max);
    return T{value};
  }

  template <typename T>
    requires EffectConcept<T>
  auto operator()(T lhs, T rhs) -> EffectType {
    auto value = lhs.value + rhs.value;
    return T{value};
  }

  auto operator()(auto lhs, auto rhs) -> EffectType {
    auto str =
        std::format("Try to update effect with different types({}, {})",
                    typeid(decltype(lhs)).name(), typeid(decltype(rhs)).name());
    const auto location = std::source_location::current();
    godot::_err_print_error(location.function_name(), location.file_name(),
                            location.line(), str.c_str());

    return lhs;
  }
};

} // namespace game

#endif