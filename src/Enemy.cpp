
#include "Enemy.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/character_body3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/path_follow3d.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/rigid_body3d.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include <algorithm>
#include <format>
#include <limits>

namespace game {

auto Enemy::_bind_methods() -> void {
  godot::ClassDB::bind_method(godot::D_METHOD("die"), &Enemy::die);
}

godot::StaticBody3D *CreateBoxEnemy(godot::Vector3 size, godot::Color color) {
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
  boxMesh->set_material(material);
  mesh3D->set_mesh(boxMesh);

  staticBody->add_child(mesh3D);

  return staticBody;
}

auto Enemy::_ready() -> void {

  godot::Ref<godot::RandomNumberGenerator> rng;
  rng.instantiate();
  auto *staticBody = CreateBoxEnemy(
      {10, 10, 10}, {rng->randfn(), rng->randfn(), rng->randfn()});

  add_child(staticBody);
  add_to_group("Enemy");

  set_gravity_scale(0);
  // set_contact_monitor(true);
  // set_max_contacts_reported(std::numeric_limits<int32_t>::max());
}

auto Enemy::_physics_process(double delta) -> void {
  if (path) {
    auto position = get_global_position();

    const auto &wayPoints = path->get();
    auto target = wayPoints[targetWayPointIndex];
    if (position.distance_squared_to(target) < 1) {
      targetWayPointIndex =
          std::clamp(++targetWayPointIndex, std::size_t{0},
                     static_cast<std::size_t>(wayPoints.size() - 1));
      target = wayPoints[targetWayPointIndex];
    }
    auto velocity = (target - position).normalized() * speed * delta;
    move_and_collide(velocity);
  }
}

auto Enemy::die() -> void {
  // auto str = std::format("Die; id = {}", get_instance_id());
  // godot::_err_print_error("on_enemy_entered", "", 0, str.c_str());
  get_parent()->remove_child(this);
  queue_free();
}

auto Enemy::SetPath(const godot::PackedVector3Array &path) -> void {
  this->path = std::ref(path);
}

auto Enemy::GetRemainDistance() const -> double {
  const auto &wayPoints = path->get();
  auto position = get_global_position();

  double remainDistance{};
  for (auto i = targetWayPointIndex; i < wayPoints.size(); ++i) {
    auto target = wayPoints[i];
    remainDistance += position.distance_to(target);
    position = target;
  }
  return remainDistance;
}
} // namespace game