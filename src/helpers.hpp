
#ifndef HELPERSHPP
#define HELPERSHPP

#include "godot_cpp/classes/area3d.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <source_location>
#include <string>

namespace debug {

inline auto PrintError(
    const std::string &error,
    const std::source_location location = std::source_location::current()) {
  godot::_err_print_error(location.function_name(), location.file_name(),
                          location.line(), error.c_str());
}

} // namespace debug

namespace game {

inline godot::CollisionShape3D *CreateCollisionBox(godot::Vector3 size) {
  auto collision3D = memnew(godot::CollisionShape3D);
  auto shape3D = godot::Ref<godot::BoxShape3D>{};
  shape3D.instantiate();
  shape3D->set_size(size);
  collision3D->set_shape(shape3D);

  return collision3D;
}

inline godot::Area3D *CreateAreaBox(godot::Vector3 size) {
  auto area3D = memnew(godot::Area3D);

  auto collision3D = CreateCollisionBox(size);
  area3D->add_child(collision3D);

  return area3D;
}
} // namespace game

#endif