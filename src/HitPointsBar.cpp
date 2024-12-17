
#include "HitPointsBar.hpp"
#include "Enemies.hpp"
#include "godot_cpp/classes/quad_mesh.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/shader_material.hpp"

namespace game {

auto HitPointsBar::_bind_methods() -> void {}

auto HitPointsBar::_ready() -> void {
  auto quadMesh = godot::Ref<godot::QuadMesh>{};
  quadMesh.instantiate();
  quadMesh->set_size({10, 1}); // TODO mv to constructor

  shaderMaterial.instantiate();
  auto *loader = godot::ResourceLoader::get_singleton();
  auto res = loader->load("res://shaders/health_bar.gdshader");
  shaderMaterial->set_shader(res);

  quadMesh->set_material(shaderMaterial);

  set_mesh(quadMesh);

  auto allPoints = hp.health + hp.armor + hp.shield;
  auto healthRatio = static_cast<double>(hp.health) / allPoints;
  auto armorRatio = static_cast<double>(hp.armor) / allPoints;
  auto shieldRatio = static_cast<double>(hp.shield) / allPoints;
  shaderMaterial->set_shader_parameter("health", healthRatio);
  shaderMaterial->set_shader_parameter("armor", healthRatio + armorRatio);
  shaderMaterial->set_shader_parameter("shield",
                                       healthRatio + armorRatio + shieldRatio);
}

//- ----------

HitPointsBar::HitPointsBar(HitPoints hp) : hp(hp) {}

auto HitPointsBar::Update(HitPoints hp) -> void {
  auto healthRatio = static_cast<double>(hp.health) / this->hp.health;
  auto armorRatio = static_cast<double>(hp.armor) / this->hp.armor;
  auto shieldRatio = static_cast<double>(hp.shield) / this->hp.shield;

  auto health = shaderMaterial->get_shader_parameter("health");
  auto armor = shaderMaterial->get_shader_parameter("armor");
  auto shield = shaderMaterial->get_shader_parameter("shield");

  shaderMaterial->set_shader_parameter("health", healthRatio * double(health));
  shaderMaterial->set_shader_parameter("armor", (healthRatio + armorRatio) *
                                                    double(armor));
  shaderMaterial->set_shader_parameter(
      "shield", (healthRatio + armorRatio + shieldRatio) * double(shield));
}

} // namespace game