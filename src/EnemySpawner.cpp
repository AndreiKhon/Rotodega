
#include "EnemySpawner.hpp"
#include "Enemy.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/vector3.hpp"

namespace game {

auto EnemySpawner::_bind_methods() -> void {
  godot::ClassDB::bind_method(godot::D_METHOD("on_timer"),
                              &EnemySpawner::on_timer);
}

auto EnemySpawner::_ready() -> void {
  auto *timer = memnew(godot::Timer);
  timer->set_wait_time(1);
  timer->connect("timeout", godot::Callable(this, "on_timer"));
  add_child(timer);
  timer->start();
}

auto EnemySpawner::on_timer() -> void {
  if (!enemies.empty()) {
    auto enemyType = enemies.back();
    auto *enemy = memnew(Enemy(enemyType));
    enemies.resize(enemies.size() - 1);
    enemy->SetPath(path);

    add_child(enemy);
    enemy->set_global_position(path[0]);
  }
}

auto EnemySpawner::SetPosition(godot::Vector3 position) -> void {
  this->position = position;
}

auto EnemySpawner::GetPosition() -> godot::Vector3 { return position; }

auto EnemySpawner::SetEnemies(const EnemiesVector& enemies) -> void {
  this->enemies = enemies;
}

auto EnemySpawner::SetPath(godot::PackedVector3Array path) -> void {
  this->path = path;
}

auto EnemySpawner::GetPath() -> godot::PackedVector3Array { return {}; }

} // namespace game