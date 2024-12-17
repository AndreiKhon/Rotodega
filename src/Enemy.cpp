
#include "Enemy.hpp"
#include "HitPointsBar.hpp"
#include "StatusEffect.hpp"
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
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <format>
#include <limits>
#include <variant>

namespace game {

auto Enemy::_bind_methods() -> void {
  godot::ClassDB::bind_method(godot::D_METHOD("die"), &Enemy::die);
  godot::ClassDB::bind_method(godot::D_METHOD("on_effect_deplete"),
                              &Enemy::on_effect_deplete);
  godot::ClassDB::bind_method(godot::D_METHOD("on_effect_update"),
                              &Enemy::on_effect_update);
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
  
  auto hp = std::visit(GetEnemyHPVisitor{}, enemy);
  hpBar = memnew(HitPointsBar(hp));
  staticBody->add_child(hpBar);
  hpBar->set_position(godot::Vector3{0, 20, 0});
}

auto Enemy::_physics_process(double delta) -> void {
  if (path) {
    auto position = get_global_position();

    const auto &wayPoints = path->get();
    auto target = wayPoints[targetWayPointIndex];
    if (position.distance_squared_to(target) < 1) {
      targetWayPointIndex =
          std::clamp(targetWayPointIndex + 1, std::size_t{0},
                     static_cast<std::size_t>(wayPoints.size() - 1));
      target = wayPoints[targetWayPointIndex];
    }
    modifiedSpeed = std::visit(GetEnemySpeedVisitor{}, enemy);
    auto velocity = (target - position).normalized() * modifiedSpeed * delta;
    move_and_collide(velocity);
  }
}

auto Enemy::on_effect_deplete(std::uint64_t id) -> void {
  effectsId.erase(id);
  auto *statusEffectObj = godot::ObjectDB::get_instance(id);
  if (statusEffectObj) {
    auto *statusEffect = static_cast<StatusEffect *>(statusEffectObj);
    enemy = std::visit(SetEnemySpeedVisitor{}, enemy, std::variant<double>(originalSpeed));
    statusEffect->queue_free();
  }
}

auto Enemy::on_effect_update(std::uint64_t id) -> void {
  auto *statusEffectObj = godot::ObjectDB::get_instance(id);
  if (statusEffectObj) {
    auto *statusEffect = static_cast<StatusEffect *>(statusEffectObj);
    auto effect = statusEffect->GetType();
    enemy = std::visit(SetEnemySpeedVisitor{}, enemy, std::variant<double>(originalSpeed));
    enemy = std::visit(ApplyEffectOnEnemy{}, enemy, effect);
    auto speed = std::visit(GetEnemySpeedVisitor{}, enemy);
    debug::PrintError(std::format("applied effect {}", speed));
  }
}

auto Enemy::die() -> void {
  // auto str = std::format("Die; id = {}", get_instance_id());
  // godot::_err_print_error("on_enemy_entered", "", 0, str.c_str());
  get_parent()->remove_child(this);
  queue_free();
}

// ---------------------------

Enemy::Enemy(EnemyType enemy) {
  this->enemy = enemy;
  originalSpeed = std::visit(GetEnemySpeedVisitor{}, enemy);
}

auto Enemy::SetPath(const godot::PackedVector3Array &path) -> void {
  this->path = std::ref(path);
}

auto Enemy::GetRemainDistance() const -> double {
  const auto &wayPoints = path->get();
  auto position = get_global_position();

  double remainDistance{};
  for (int64_t i = targetWayPointIndex; i < wayPoints.size(); ++i) {
    auto target = wayPoints[i];
    remainDistance += position.distance_to(target);
    position = target;
  }
  return remainDistance;
}

auto Enemy::GetType() const -> EnemyType { return enemy; }

auto Enemy::Apply(EffectType effect) -> void {
  for (auto statusEffectId : effectsId) {
    auto *statusEffectObj = godot::ObjectDB::get_instance(statusEffectId);
    if (!statusEffectObj) {
      continue;
    }

    auto *statusEffect = static_cast<StatusEffect *>(statusEffectObj);
    auto existingEffect = statusEffect->GetType();
    auto isSame = std::visit(IsSameEffectVisitor{}, existingEffect, effect);
    if (isSame) {
      statusEffect->Update(effect);
      return;
    }
  }

  // Create new StatusEffect
  auto *statusEffect = memnew(StatusEffect);
  statusEffect->SetType(effect);
  statusEffect->connect("effect_depleted",
                        godot::Callable(this, "on_effect_deplete"));
  statusEffect->connect("effect_updated",
                        godot::Callable(this, "on_effect_update"));
  add_child(statusEffect);
  effectsId.emplace(statusEffect->get_instance_id());
}

} // namespace game