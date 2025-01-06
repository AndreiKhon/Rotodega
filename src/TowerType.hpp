
#ifndef TOWERTYPEHPP
#define TOWERTYPEHPP

#include "StatusEffect.hpp"
#include <Towers.hpp>
#include <concepts>
#include <variant>

namespace game {

using TowerVariant = std::variant<TestTower0, TestTower1>;

class TowerType {
public:
  TowerType() = default;
  explicit TowerType(TowerVariant tower);

  auto GetType() { return tower; }

  auto AddEffect(EffectType effect) -> void;

  auto GetBaseDamage() -> std::uint64_t const;
  auto GetMultipliers() -> HPMultiplier const;
  auto GetRange() -> double const;
  auto GetRPM() -> std::uint64_t const;
  auto GetElectricity() -> std::uint64_t const;
  auto GetEffects() -> EffectVector const;
  auto GetCrits() -> CriticalChance const;

  auto GetModelPath() -> const char* const;

private:
  TowerVariant tower;
};

template <typename T>
concept BasicTower = requires(T tower) {
  { tower.baseDamage } -> std::convertible_to<std::uint64_t>;
  { tower.multipliers } -> std::convertible_to<HPMultiplier>;
  { tower.range } -> std::convertible_to<double>;
  { tower.rpm } -> std::convertible_to<std::uint64_t>;
  { tower.electricity } -> std::convertible_to<std::uint64_t>;
  { tower.effects } -> std::convertible_to<EffectVector>;
  { tower.crits } -> std::convertible_to<CriticalChance>;
};

// Check if all TowerType alternatives have BasicTower features at compile time
// -------------------------
template <std::size_t... i> inline void TowerTest(std::index_sequence<i...>) {
  static_assert((BasicTower<decltype(std::get<i>(TowerVariant{}))> && ...));
}

inline void TowersTest() {
  constexpr auto size = std::variant_size_v<TowerVariant>;
  TowerTest(std::make_index_sequence<size>{});
}
// ------------------------------------------------------------------------------------------------

} // namespace game

#endif