
#include "Enemy.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/path3d.hpp"
#include "godot_cpp/classes/path_follow3d.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/curve3d.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"

namespace game {

auto Enemy::_bind_methods() -> void {}

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
  // material->set_transparency( godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
  boxMesh->set_material(material);
  mesh3D->set_mesh(boxMesh);

  staticBody->add_child(mesh3D);

  return staticBody;
}

auto Enemy::_ready() -> void {
  pathFollow = memnew(godot::PathFollow3D);
    pathFollow->set_loop(false);
    godot::Ref<godot::RandomNumberGenerator> rng;
    rng.instantiate();
  auto *staticBody = CreateBoxEnemy({10, 10, 10}, {rng->randfn(), rng->randfn(), rng->randfn()});
  pathFollow->add_child(staticBody);

  add_child(pathFollow);
}

auto Enemy::_process(double delta) -> void {
    auto distance = speed * delta;
    pathFollow->set_progress(pathFollow->get_progress() + distance);
}

auto Enemy::SetPathCurve(godot::Ref<godot::Curve3D> path) -> void {
    set_curve(path);
}

} // namespace game