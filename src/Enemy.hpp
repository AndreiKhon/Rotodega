
#ifndef ENEMYHPP
#define ENEMYHPP

#include "Enemies.hpp"
#include "StatusEffect.hpp"
#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <functional>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <optional>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <concepts>

namespace game {

template <typename T>
concept BasicEnemy = requires(T enemy) {
  { enemy.hp } -> std::convertible_to<HitPoints>;

  { enemy.damage } -> std::convertible_to<std::uint64_t>;
  { enemy.scraps } -> std::convertible_to<std::uint64_t>;

  { enemy.speed } -> std::convertible_to<double>;
};

using EnemyType = std::variant<TestEnemy0, TestEnemy1, TestEnemy2>;

// Check if all EnemyType alternatives have BasicEnemy features at compile time
// -------------------------
template <std::size_t... i> inline void EnemyTest(std::index_sequence<i...>) {
  static_assert((BasicEnemy<decltype(std::get<i>(EnemyType{}))> && ...));
}

inline void EnemiesTest() {
  constexpr auto size = std::variant_size_v<EnemyType>;
  EnemyTest(std::make_index_sequence<size>{});
}
// ------------------------------------------------------------------------------------------------

using EnemiesVector = std::vector<EnemyType>; // TODO span

class Enemy : public godot::RigidBody3D {
  GDCLASS(Enemy, godot::RigidBody3D)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _physics_process(double delta) -> void override;

  auto on_effect_deplete(uint64_t id) -> void;
  auto on_effect_update(uint64_t id) -> void;

  auto die() -> void;

public:
  auto SetPath(const godot::PackedVector3Array &path) -> void;

  auto GetRemainDistance() const -> double;

  auto GetType() const -> EnemyType;
  auto SetType(EnemyType) -> void;

  auto Apply(EffectType effect) -> void;

private:
  EnemyType enemy;
  std::unordered_set<uint64_t> effectsId;
  std::optional<std::reference_wrapper<const godot::PackedVector3Array>> path;
  std::size_t targetWayPointIndex = 0;
  double modifiedSpeed;
  double originalSpeed;
};

// Visitors

struct ApplyEffectOnEnemy {
  auto operator()(BasicEnemy auto enemy, Slow effect) -> EnemyType {
    enemy.speed *= (100.0 - effect.value) / 100.0;
    return enemy;
  }
  auto operator()(BasicEnemy auto enemy, Haste effect) -> EnemyType {
    enemy.speed *= (100.0 + effect.value) / 100;
    return enemy;
  }
  auto operator()(BasicEnemy auto enemy, Bleed effect) -> EnemyType {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete;
      return enemy;
    }
    return {};
  }
  auto operator()(BasicEnemy auto enemy, Burn effect) -> EnemyType {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete / 2;
      return enemy;
    }
    return {};
  }
  auto operator()(BasicEnemy auto enemy, Poison effect) -> EnemyType {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete / 2;
      return enemy;
    }
    return {};
  }
};

struct GetEnemySpeedVisitor {
  auto operator()(BasicEnemy auto enemy) -> double { return enemy.speed; }
};

struct SetEnemySpeedVisitor {
  auto operator()(BasicEnemy auto enemy,
                  std::variant<double> speed) -> EnemyType {
    enemy.speed = std::get<0>(speed);
    return enemy;
  }
};

} // namespace game

#endif