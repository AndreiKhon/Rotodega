
#include "Camera.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/collision_object3d.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/physics_direct_space_state3d.hpp"
#include "godot_cpp/classes/physics_ray_query_parameters3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/world3d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <algorithm>

namespace game {

// Godot stuff ----------------------------------------------------

auto Camera::_bind_methods() -> void {
  godot::ClassDB::bind_method(godot::D_METHOD("get_speed"), &Camera::get_speed);
  godot::ClassDB::bind_method(godot::D_METHOD("set_speed", "p_speed"),
                              &Camera::set_speed);

  ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "speed"), "set_speed",
               "get_speed");

  ADD_SIGNAL(godot::MethodInfo("geometry_requested",
                        godot::PropertyInfo(godot::Variant::VECTOR3, "position"),
                        godot::PropertyInfo(godot::Variant::VECTOR3, "size")));
}

auto Camera::_ready() -> void {
  constexpr auto pi = 3.14159;
  set_rotation(godot::Vector3{-pi / 4, -3 * pi / 4, 0});
  set_position({0, 100, 0});
}

auto Camera::_process(double delta) -> void {

  auto *input = godot::Input::get_singleton();
  auto direction = godot::Vector3{};
  // TODO Add gamepad etc
  if (input->is_key_pressed(godot::Key::KEY_W)) {
    direction.x += 1;
    direction.z += 1;
  }
  if (input->is_key_pressed(godot::Key::KEY_A)) {
    direction.x += 1;
    direction.z -= 1;
  }
  if (input->is_key_pressed(godot::Key::KEY_S)) {
    direction.x -= 1;
    direction.z -= 1;
  }
  if (input->is_key_pressed(godot::Key::KEY_D)) {
    direction.x -= 1;
    direction.z += 1;
  }
  if(input->is_key_pressed(godot::Key::KEY_CTRL)) {
    direction.y -= 1;
  }
  if(input->is_key_pressed(godot::Key::KEY_SPACE)) {
    direction.y += 1;
  }
  auto speedMultiplier = 1;
  if(input->is_key_pressed(godot::Key::KEY_SHIFT)) {
    speedMultiplier = 2;
  }
  auto velocity = direction * speed * speedMultiplier;
  auto newPosition = get_position() + velocity * delta;

  newPosition.y = std::clamp(newPosition.y, 140.0f, 1400.0f);

  set_position(newPosition);

  //   if (input->is_mouse_button_pressed(godot::MOUSE_BUTTON_WHEEL_UP)) {
  //     auto fov = get_fov();
  //     fov -= 5.0f;
  //     fov = std::clamp(fov, 25.0f, 125.0f);
  //     set_fov(fov);
  //   }
  //   if (input->is_mouse_button_pressed(godot::MOUSE_BUTTON_WHEEL_DOWN)) {
  //     auto fov = get_fov();
  //     fov += 5.0f;
  //     fov = std::clamp(fov, 25.0f, 125.0f);
  //     set_fov(fov);
  //   }
}

auto Camera::_unhandled_input(const godot::Ref<godot::InputEvent> &p_event)
    -> void {

  if (p_event->is_class("InputEventMouseButton")) {
    godot::Ref<godot::InputEventMouseButton> mbe = p_event;
    if (mbe->is_pressed()) {
      auto buttonIndex = mbe->get_button_index();
      switch (buttonIndex) {
      case godot::MOUSE_BUTTON_LEFT:
        get_selection(mbe->get_position());
        break;
      case godot::MOUSE_BUTTON_RIGHT:
        get_geometry(mbe->get_position());
        break;
      case godot::MOUSE_BUTTON_WHEEL_UP: {
        auto fov = get_fov();
        fov -= 5.0f;
        fov = std::clamp(fov, 25.0f, 125.0f);
        set_fov(fov);
      } break;
      case godot::MOUSE_BUTTON_WHEEL_DOWN: {
        auto fov = get_fov();
        fov += 5.0f;
        fov = std::clamp(fov, 25.0f, 125.0f);
        set_fov(fov);
      } break;
      default:
        break;
      }
    }
  }
}

auto Camera::get_selection(const godot::Vector2 &mousePos) -> void {
  auto worldspace = get_world_3d()->get_direct_space_state();
  auto start = project_ray_origin(mousePos);
  auto end = project_position(mousePos, 1500);
  auto result = worldspace->intersect_ray(
      godot::PhysicsRayQueryParameters3D::create(start, end));
  if (!result.is_empty()) {
    if (result.has("collider")) {
      auto collider = result["collider"];

      auto *staticbody =
          dynamic_cast<godot::StaticBody3D *>(static_cast<godot::Object *>(collider));
      if (staticbody) {
        auto parent = staticbody->get_parent();
        if (parent->has_method("interact")) {
          parent->call("interact");
        }
      }
    }
  }
}

auto Camera::get_geometry(const godot::Vector2 &mousePos) -> void {
  auto worldspace = get_world_3d()->get_direct_space_state();
  auto start = project_ray_origin(mousePos);
  auto end = project_position(mousePos, 1500);
  auto result = worldspace->intersect_ray(
      godot::PhysicsRayQueryParameters3D::create(start, end));
  if (!result.is_empty()) {
    if (result.has("collider")) {
      auto collider = result["collider"];

      godot::StaticBody3D *obj =
          dynamic_cast<godot::StaticBody3D *>(static_cast<godot::Object *>(collider));
      if (obj) { // TODO Looks strange, should it be easier?
        auto parent = obj->get_parent();
        auto meshInstance =
            dynamic_cast<godot::MeshInstance3D *>(parent->get_child(1));
        if (meshInstance) {
          godot::Ref<godot::BoxMesh> mesh = meshInstance->get_mesh(); // TODO Works only with boxes
          auto position = obj->get_global_position();
          auto size = mesh->get_size();

          emit_signal("geometry_requested", position, size);
        }
      }
    }
  }
}

auto Camera::get_speed() -> double { return speed; }

auto Camera::set_speed(double p_speed) -> void { speed = p_speed; }

//-----------------------------------------------------------------

} // namespace game