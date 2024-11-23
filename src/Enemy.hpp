
#ifndef ENEMYHPP
#define ENEMYHPP

#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/path_follow3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <utility>
#include <variant>
#include <vector>

#include <concepts>

namespace game {

template <typename T>
concept EnemyFeatures = requires(T enemy) {
  { enemy.health } -> std::convertible_to<std::uint64_t>;
  { enemy.armor } -> std::convertible_to<std::uint64_t>;
  { enemy.shield } -> std::convertible_to<std::uint64_t>;
  { enemy.speed } -> std::convertible_to<double>;
};

struct TestEnemy {
  std::uint64_t health;
  std::uint64_t armor;
  std::uint64_t shield;
  double speed;
};

using EnemyType = std::variant<TestEnemy>;

// Check if all EnemyType alternatives have EnemyFeatures at compile time -------------------------
template <std::size_t... i>
inline void EnemyTest(std::index_sequence<i...>) {
  static_assert((EnemyFeatures<decltype(std::get<i>(EnemyType{}))> && ...));
}

inline void EnemiesTest() {
  constexpr auto size = std::variant_size_v<EnemyType>;
  EnemyTest(std::make_index_sequence<size>{});
}
// ------------------------------------------------------------------------------------------------

using EnemiesVector = std::vector<EnemyType>;

class Enemy : public godot::Path3D {
  GDCLASS(Enemy, godot::Node)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _process(double delta) -> void override;

public:
  auto SetPathCurve(godot::Ref<godot::Curve3D> path) -> void;

private:
  EnemyType type;
  godot::PackedVector3Array path;
  godot::PathFollow3D *pathFollow;
  double speed = 50; // TODO remove
};

} // namespace game

#endif