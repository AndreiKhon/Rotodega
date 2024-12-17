
#ifndef HITPOINTSBARHPP
#define HITPOINTSBARHPP

#include "Enemies.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/shader_material.hpp"

namespace game {

class HitPointsBar : public godot::MeshInstance3D {
  GDCLASS(HitPointsBar, godot::MeshInstance3D)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;

public:
  HitPointsBar() = default;
  explicit HitPointsBar(HitPoints hp);
  auto Update(HitPoints hp) -> void;

private:
  HitPoints hp{};
  godot::Ref<godot::ShaderMaterial> shaderMaterial;
};

} // namespace game

#endif