
#ifndef ENEMYHPP
#define ENEMYHPP

#include <godot_cpp/classes/physics_body3d.hpp>

#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <functional>
#include <optional>
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

using EnemiesVector = std::vector<EnemyType>; // TODO span

class Enemy : public godot::RigidBody3D {
  GDCLASS(Enemy, godot::RigidBody3D)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _physics_process(double delta) -> void override;

  auto die() -> void;

public:
  auto SetPath(const godot::PackedVector3Array &path) -> void;

  auto GetRemainDistance() const -> double;

private:
  EnemyType type;
  std::optional<std::reference_wrapper<const godot::PackedVector3Array>> path;
  std::size_t targetWayPointIndex = 0;
  double speed = 25; // TODO remove
};

} // namespace game

#endif