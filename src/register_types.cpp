
#include "register_types.hpp"

#include "Camera.hpp"
#include "EnemySpawner.hpp"
#include "MapGenerator.hpp"
#include "Tower.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

void initialize_game_module(godot::ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }

  GDREGISTER_CLASS(game::MapGenerator);
  GDREGISTER_CLASS(game::Camera);
  GDREGISTER_CLASS(game::ExtendTile);
  GDREGISTER_CLASS(game::EnemySpawner);
  GDREGISTER_CLASS(game::Enemy);
  GDREGISTER_CLASS(game::Tower);
}

void uninitialize_game_module(godot::ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT
game_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                  const GDExtensionClassLibraryPtr p_library,
                  GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_game_module);
  init_obj.register_terminator(uninitialize_game_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}