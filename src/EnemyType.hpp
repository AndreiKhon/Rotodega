
#ifndef ENEMYTYPEHPP
#define ENEMYTYPEHPP

#include "Enemies.hpp"
#include "StatusEffect.hpp"

namespace game {

using EnemyVariant = std::variant<TestEnemy0, TestEnemy1, TestEnemy2>;
using EnemiesVector = std::vector<EnemyVariant>; // TODO span

class EnemyType {
public:
  EnemyType() = default;
  explicit EnemyType(EnemyVariant enemy);

  auto GetHP() -> HitPoints const;

  auto GetSpeed() -> double const;
  auto SetSpeed(double speed) -> void;

  auto ApplyEffect(EffectType effect) -> void;

private:
  EnemyVariant enemy;
};

template <typename T>
concept BasicEnemy = requires(T enemy) {
  { enemy.hp } -> std::convertible_to<HitPoints>;

  { enemy.damage } -> std::convertible_to<std::uint64_t>;
  { enemy.scraps } -> std::convertible_to<std::uint64_t>;

  { enemy.speed } -> std::convertible_to<double>;
};

// Check if all EnemyType alternatives have BasicEnemy features at compile time
// -------------------------
template <std::size_t... i> inline void EnemyTest(std::index_sequence<i...>) {
  static_assert((BasicEnemy<decltype(std::get<i>(EnemyVariant{}))> && ...));
}

inline void EnemiesTest() {
  constexpr auto size = std::variant_size_v<EnemyVariant>;
  EnemyTest(std::make_index_sequence<size>{});
}
// ------------------------------------------------------------------------------------------------

} // namespace game

#endif