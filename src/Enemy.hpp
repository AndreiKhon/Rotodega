
#ifndef ENEMYHPP
#define ENEMYHPP

#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/path_follow3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <utility>
#include <variant>
#include <vector>

namespace game {

struct TestEnemy {};

using EnemyType = std::variant<TestEnemy>;

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