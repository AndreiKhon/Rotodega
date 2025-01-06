
#include "Tower.hpp"
#include "TowerType.hpp"
#include "Towers.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/sphere_shape3d.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "helpers.hpp"
#include <functional>
#include <limits>
#include <optional>

namespace game {

auto Tower::_bind_methods() -> void {
  godot::ClassDB::bind_method(godot::D_METHOD("on_area_entered", "enemy"),
                              &Tower::on_area_entered);
  godot::ClassDB::bind_method(godot::D_METHOD("on_area_exited", "enemy"),
                              &Tower::on_area_exit);

  godot::ClassDB::bind_method(godot::D_METHOD("on_reload_finished"),
                              &Tower::on_reload_finished);
}

godot::StaticBody3D *CreateTowerBox(godot::Vector3 size, godot::Color color) {
  auto staticBody = memnew(godot::StaticBody3D);

  auto mesh3D = memnew(godot::MeshInstance3D);
  auto boxMesh = godot::Ref<godot::BoxMesh>{};
  boxMesh.instantiate();
  boxMesh->set_size(size);
  auto material = godot::Ref<godot::StandardMaterial3D>{};
  material.instantiate();
  material->set_albedo(color);
  boxMesh->set_material(material);
  mesh3D->set_mesh(boxMesh);

  staticBody->add_child(mesh3D);

  return staticBody;
}

Tower::Tower(TowerVariant tower) : tower(tower) {}

struct TowerBoxVisitor {
  auto operator()(TestTower0 tower) -> godot::StaticBody3D * {
    return CreateTowerBox(godot::Vector3{20, 20, 20}, godot::Color{1, 0, 1});
  };
  auto operator()(TestTower1 tower) -> godot::StaticBody3D * {
    return CreateTowerBox(godot::Vector3{20, 20, 20}, godot::Color{0, 1, 1});
  };
};

auto Tower::_ready() -> void {
  connect("area_entered", godot::Callable(this, "on_area_entered"));
  connect("area_exited", godot::Callable(this, "on_area_exited"));

  constexpr float radius = 100;

  auto *collision3D = memnew(godot::CollisionShape3D);
  auto shape3D = godot::Ref<godot::SphereShape3D>{};
  shape3D.instantiate();
  shape3D->set_radius(radius);
  collision3D->set_shape(shape3D);
  add_child(collision3D);

  auto *loader = godot::ResourceLoader::get_singleton();
  auto modelPath = tower.GetModelPath();
  godot::Ref<godot::PackedScene> model = loader->load(modelPath);
  add_child(model->instantiate());

  reloadTimer = memnew(godot::Timer);
  reloadTimer->set_wait_time(1);
  reloadTimer->set_one_shot(true);
  reloadTimer->connect("timeout", godot::Callable(this, "on_reload_finished"));

  add_child(reloadTimer);

  set_monitoring(true);
}

auto Tower::_physics_process(double delta) -> void {
  set_global_position(get_global_position()); // ?????????

  SelectTarget(); // set minimum delay before next SelectTarget ?
  // look_at ?
  if (needReload == true) {
    return;
  }
  ShootTarget();
}

auto Tower::on_area_entered(godot::Node3D *area) -> void {
  auto *parent = area->get_parent();
  if (parent->is_class("Enemy")) {
    auto id = parent->get_instance_id();
    enemiesInArea.emplace(id);
  }
}

auto Tower::on_area_exit(godot::Node3D *body) -> void {
  auto *parent = body->get_parent();
  if (parent->is_class("Enemy")) {
    auto id = parent->get_instance_id();
    enemiesInArea.erase(id);
  }
}

auto Tower::on_reload_finished() -> void { needReload = false; }

auto Tower::SelectTarget() -> void {
  auto minRemainDistance = std::numeric_limits<double>::max();

  target = std::nullopt;

  for (auto id : enemiesInArea) {
    Enemy *enemy = static_cast<Enemy *>(
        godot::ObjectDB::get_instance(id)); // TODO check if too slow
    if (!enemy) {
      continue;
    }

    auto remain = enemy->GetRemainDistance();
    if (remain < minRemainDistance) {
      minRemainDistance = remain;
      target = *enemy;
    }
  }
}
auto Tower::ShootTarget() -> void {
  if (target) {
    auto &enemy = target->get();
    enemy.call("die");
    SetReload();
  }
}

auto Tower::PredictTargetPosition() -> void {}

auto Tower::SetReload() -> void {
  needReload = true;
  reloadTimer->start();
}

} // namespace game