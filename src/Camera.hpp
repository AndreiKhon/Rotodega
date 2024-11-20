
#ifndef CAMERAHPP
#define CAMERAHPP

#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/classes/input_event.hpp"

namespace game {

class Camera : public godot::Camera3D {
  GDCLASS(Camera, godot::Camera3D)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _process(double) -> void override;
  auto _unhandled_input(const godot::Ref<godot::InputEvent> &p_event) -> void override;

  auto get_selection(const godot::Vector2&) -> void;

public:
  auto get_speed() -> double;
  auto set_speed(double) -> void;

private:
  double speed = 75;
};

} // namespace game

#endif