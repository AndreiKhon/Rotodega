
#ifndef TOWERHPP
#define TOWERHPP

#include "Enemy.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/timer.hpp"
#include <functional>
#include <godot_cpp/classes/area3d.hpp>
#include <optional>
#include <unordered_set>

namespace game {

class Tower : public godot::Area3D {
  GDCLASS(Tower, godot::Area3D)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _physics_process(double delta) -> void override;


private:
auto on_enemy_entered(godot::Node3D* enemy) -> void;
auto on_enemy_exit(godot::Node3D* enemy) -> void;

auto on_reload_finished() -> void;

auto SelectTarget() -> void;
auto ShootTarget() -> void;
auto PredictTargetPosition() -> void;
auto SetReload() -> void;


private:
godot::Timer* reloadTimer;

private:
std::unordered_set<std::uint64_t> enemiesInArea;

std::optional<std::reference_wrapper<Enemy>> target;
bool needReload = false;
};

} // namespace game

#endif