

#ifndef MAPGENERATORHPP
#define MAPGENERATORHPP

#include "Enemy.hpp"
#include "EnemySpawner.hpp"
#include "Map.hpp"
#include "Tile.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/static_body2d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <cstddef>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <random>
#include <vector>

namespace game {

class ExtendTile;

class MapGenerator : public godot::Node {
  GDCLASS(MapGenerator, Node)
protected:
  static void _bind_methods();

private:
  void PrepairTileFor3D(Position pos);

public:
  void _ready() override;
  void extend_pressed(godot::PackedInt64Array, int);

private:
  Map map;
  // std::vector<Position> tilesForExtension;
  static constexpr std::size_t maxTilesCount =
      15; // TODO check why generated less tiles by 1
  std::size_t extendTilesCount = 0;
  std::size_t spawnersCount = 0;
  // DirectionsVector startDirections;
  std::size_t startDirectionsCount = 1;

  std::random_device randomDevice;
  std::mt19937 generator{randomDevice()};
  std::uniform_int_distribution<> distribution{1, 100};

public:
  static std::uint8_t cellSize;

public:
  auto SetStartDirectionsCount(std::size_t startDirectionsCount) -> void;
  auto GetStartDirectionsCount() -> std::size_t;
  void Extend(Position position, Direction direction);

private:
  void AddStartTile();
  Tile GenerateStartTile(DirectionsVector directions);
  Tile GenerateTile(Direction extendFrom, DirectionsVector possibleExtensions);
  void AddRoad(Tile &tile,
               Direction direction); // TODO make better generation sistem
  auto AddRoads(Tile &tile, DirectionsVector directions) -> void;
  bool IsTileForExtention(Position position);
  DirectionsVector GetFreePositionsAround(Position position);
  auto GetFreeTilesCount() const -> std::size_t;
  bool HasRoad(Position position, Direction direction);
  std::size_t RoadsCount(Position position);
  DirectionsVector GetAllRoads(Position position);
  auto GetExtendRoads(Position , Direction extendFrom) -> DirectionsVector;

  Position Move(Position position, Direction direction);
  Direction GetOppositeDirection(Direction direction);

  ExtendTile *CreateExtendTile(Position pos, Direction direction);
  auto AddExtendTiles(Position position, DirectionsVector directions) -> void;

  godot::Vector3
  calculateTilePosition3D(Position position); // Shoul be in an another class?

  auto CreateEnemySpawner() -> EnemySpawner*;
  auto AddEnemySpawners(Position position, DirectionsVector directions) -> void;
  auto AddEnemySpawnerForPortal(Position position) -> void;
  auto CalculateEnemySpawnerPosition(Position position, Direction direction) -> godot::Vector3;
  auto UpdateEnemySpawnersEnemies(const EnemiesVector& enemies) -> void;
  auto DeleteNearestSpawner(godot::Vector3 position) -> void;
};

// class ExtendButton : public godot::Button {
//   GDCLASS(ExtendButton, Button)

// protected:
//   static auto _bind_methods() -> void;

// public:
//   auto _ready() -> void override;
//   auto _process(double) -> void override;

//   public:
//   auto SetExtendFromPosition(Position) -> void;
//   auto SetDirection(Direction) -> void;

// private:
//   Position extendFromPosition;
//   Direction direction;
// };

class ExtendTile : public godot::StaticBody3D {
  GDCLASS(ExtendTile, godot::StaticBody3D)

protected:
  static auto _bind_methods() -> void;

public:
  auto _ready() -> void override;
  auto _process(double) -> void override;

  auto interact() -> void;

public:
  auto SetExtendFromPosition(Position) -> void;
  auto SetDirection(Direction) -> void;

private:
  Position extendFromPosition;
  Direction direction;
};

} // namespace game

#endif