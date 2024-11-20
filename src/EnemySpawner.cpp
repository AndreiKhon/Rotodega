
#include "EnemySpawner.hpp"
#include "Enemy.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "godot_cpp/variant/callable.hpp"
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

godot::StaticBody3D *TestCreateBox(godot::Vector3 size, godot::Color color) {
  auto staticBody = memnew(godot::StaticBody3D);

  auto collision3D = memnew(godot::CollisionShape3D);
  auto shape3D = godot::Ref<godot::BoxShape3D>{};
  shape3D.instantiate();
  shape3D->set_size(size);
  collision3D->set_shape(shape3D);

  staticBody->add_child(collision3D);

  auto mesh3D = memnew(godot::MeshInstance3D);
  auto boxMesh = godot::Ref<godot::BoxMesh>{};
  boxMesh.instantiate();
  boxMesh->set_size(size);
  auto material = godot::Ref<godot::StandardMaterial3D>{};
  material.instantiate();
  material->set_albedo(color);
  material->set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
  boxMesh->set_material(material);
  mesh3D->set_mesh(boxMesh);

  staticBody->add_child(mesh3D);

  return staticBody;
}

struct EnemyCreate {
  auto operator()(TestEnemy) -> godot::StaticBody3D * {
    return TestCreateBox(godot::Vector3{10.0f, 10.0f, 10.0f},
                         godot::Color{1, 1, 1, 0.1});
  }
};

auto EnemySpawner::on_timer() -> void {
  if (!enemies.empty()) {
    auto *enemy = std::visit(EnemyCreate{}, enemies.back());
    enemies.resize(enemies.size() - 1);

    add_child(enemy);
    enemy->set_global_position(position);
  }
}

auto EnemySpawner::SetPosition(godot::Vector3 position) -> void {
  this->position = position;
}

auto EnemySpawner::GetPosition() -> godot::Vector3 { return position; }

auto EnemySpawner::SetEnemies(EnemiesVector enemies) -> void {
  this->enemies = enemies;
}

} // namespace game