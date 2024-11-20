
#ifndef ENEMYSPAWNERHPP
#define ENEMYSPAWNERHPP

#include "Enemy.hpp"
#include "Tile.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/variant/vector3.hpp"

#include <span>

namespace game {

class EnemySpawner : public godot::Node {
  GDCLASS(EnemySpawner, godot::Node)
protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  //   auto _process(double) -> void override;

  auto on_timer() -> void;

public:
  //   auto Spawn() -> void;
  auto SetPosition(godot::Vector3 position) -> void;
  auto GetPosition() -> godot::Vector3;
  auto SetEnemies(EnemiesVector enemies) -> void;

private:
  godot::Vector3 position;
  EnemiesVector enemies;
};

} // namespace game

#endif