
#ifndef ENEMYHPP
#define ENEMYHPP

#include "EnemyType.hpp"
#include "HitPointsBar.hpp"
#include "StatusEffect.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <functional>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <optional>
#include <unordered_set>

namespace game {

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
  Enemy() = default;
  explicit Enemy(EnemyVariant enemy);
  auto SetPath(const godot::PackedVector3Array &path) -> void;

  auto GetRemainDistance() const -> double;

  auto Apply(EffectType effect) -> void;

private:
  EnemyType enemy;
  std::unordered_set<uint64_t> effectsId;
  std::optional<std::reference_wrapper<const godot::PackedVector3Array>> path;
  std::size_t targetWayPointIndex = 0;
  HitPointsBar *hpBar = nullptr;
  double modifiedSpeed{};
  double originalSpeed{};
};

} // namespace game

#endif