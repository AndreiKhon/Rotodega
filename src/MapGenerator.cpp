
#include "MapGenerator.hpp"
#include "Enemy.hpp"
#include "EnemySpawner.hpp"
#include "Tile.hpp"
#include "gdextension_interface.h"
#include "godot_cpp/classes/a_star3d.hpp"
#include "godot_cpp/classes/base_material3d.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/collision_object3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/label3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/orm_material3d.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/vector3.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <limits>
#include <ranges>
#include <string>
#include <variant>

namespace game {

// Godot stuff ------------------------------------------------------------
void MapGenerator::_bind_methods() {
  godot::ClassDB::bind_method(
      godot::D_METHOD("extend_pressed_method", "position", "direction"),
      &MapGenerator::extend_pressed);

  godot::ClassDB::bind_method(godot::D_METHOD("get_start_directions_count"),
                              &MapGenerator::GetStartDirectionsCount);
  godot::ClassDB::bind_method(
      godot::D_METHOD("set_start_directions_count", "p_start_directions_count"),
      &MapGenerator::SetStartDirectionsCount);
  ADD_PROPERTY(
      godot::PropertyInfo(godot::Variant::INT, "start_directions_count"),
      "set_start_directions_count", "get_start_directions_count");
  // godot::_err_print_error("_bind_methods", "MapGenerator.cpp", 44,
  // "Success");
}

void MapGenerator::_ready() {
  aStar.instantiate();
  AddStartTile();

  constexpr auto startPosition = Position{0, 0};
  PrepairTileFor3D(startPosition);

  auto directions = GetAllRoads(startPosition);
  AddExtendTiles(startPosition, directions);

  auto position3d = calculateTilePosition3D(startPosition);
  aStar->add_point(lastAStarId++, position3d);
  AddWayPoints(startPosition, directions);
  AddEnemySpawners(startPosition, directions, aStar);
}

void MapGenerator::extend_pressed(godot::PackedInt64Array array, int integer) {
  Extend(Position{array[0], array[1]}, static_cast<Direction>(integer));
}

godot::StaticBody3D *CreateBox(godot::Vector3 size, godot::Color color) {
  auto staticBody = memnew(godot::StaticBody3D);

  auto collision3D = memnew(godot::CollisionShape3D);
  auto shape3D = godot::Ref<BoxShape3D>{};
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

struct PrepairCellFor3DVisitor {
  godot::StaticBody3D *operator()(GrassCell gc) {
    return CreateBox(
        godot::Vector3{static_cast<real_t>(MapGenerator::cellSize),
                       static_cast<real_t>(MapGenerator::cellSize / 4.0 *
                                           (gc.elevation + 1)),
                       static_cast<real_t>(MapGenerator::cellSize)},
        godot::Color{0, 1.0f / (1 + gc.elevation), 0});
  }
  godot::StaticBody3D *operator()(RoadCell rc) { return nullptr; }
  godot::StaticBody3D *operator()(BaseCell bc) {
    return CreateBox(
        godot::Vector3{static_cast<real_t>(MapGenerator::cellSize),
                       static_cast<real_t>(MapGenerator::cellSize * 4),
                       static_cast<real_t>(MapGenerator::cellSize)},
        godot::Color{0, 0, 1, 0.5});
  }
  godot::StaticBody3D *operator()(PortalCell pc) {
    return CreateBox(
        godot::Vector3{static_cast<real_t>(MapGenerator::cellSize),
                       static_cast<real_t>(MapGenerator::cellSize * 4),
                       static_cast<real_t>(MapGenerator::cellSize)},
        godot::Color{1, 0, 0, 0.5});
  }
};

void MapGenerator::PrepairTileFor3D(Position position) {
  auto optTile = map.GetTile(position);

  constexpr auto additionalHeiht = 5;
  if (optTile) {
    auto foundationSize =
        godot::Vector3{static_cast<real_t>(Tile::LengthInCells * cellSize),
                       static_cast<real_t>(cellSize / 4.0),
                       static_cast<real_t>(Tile::LengthInCells * cellSize)};
    auto foundationColor = godot::Color{1, 0, 0};

    auto *staticBody = CreateBox(foundationSize, foundationColor);

    // Need to move Tile
    auto globalTilePosition = godot::Vector3{
        static_cast<real_t>(position.x * Tile::LengthInCells * cellSize), 0,
        static_cast<real_t>(position.y * Tile::LengthInCells * cellSize)};

    add_child(staticBody);
    staticBody->add_to_group("Tiles");
    staticBody->set_position(globalTilePosition + foundationSize / 2);

    auto tile = *optTile;
    for (std::int64_t i = 0; i < Tile::LengthInCells; ++i) {
      for (std::int64_t j = 0; j < Tile::LengthInCells; ++j) {
        Position cellPosition{j, i};
        auto cellStaticBody =
            std::visit(PrepairCellFor3DVisitor{}, tile.GetCell(cellPosition));
        if (cellStaticBody) {
          auto globalCellPosition =
              godot::Vector3{static_cast<real_t>(cellPosition.x * cellSize),
                             static_cast<real_t>(cellSize / 4.0),
                             static_cast<real_t>(cellPosition.y * cellSize)};

          staticBody->add_child(cellStaticBody);
          auto *mesh = dynamic_cast<godot::MeshInstance3D *>(
              cellStaticBody->get_child(1));
          godot::Ref<BoxMesh> meshSize = mesh->get_mesh();

          cellStaticBody->set_position(globalCellPosition +
                                       meshSize->get_size() / 2 -
                                       foundationSize / 2);
        }
      }
    }
  }
}

ExtendTile *MapGenerator::CreateExtendTile(Position position,
                                           Direction direction) {
  auto *extendTile = memnew(ExtendTile);
  extendTile->SetExtendFromPosition(position);
  extendTile->SetDirection(direction);
  extendTile->connect("extend_pressed",
                      godot::Callable(this, "extend_pressed_method"));
  extendTile->add_to_group("Extends");
  return extendTile;
}

auto MapGenerator::AddExtendTiles(Position position,
                                  DirectionsVector directions) -> void {
  for (auto direction : directions) {
    auto *extendTile = CreateExtendTile(position, direction);
    auto neighbor = Move(position, direction);
    auto position3d = calculateTilePosition3D(neighbor);
    add_child(extendTile);
    extendTile->set_global_position(position3d);
    extendTilesCount += 1;
  }
}

godot::Vector3 MapGenerator::calculateTilePosition3D(Position position) {
  auto globalTilePosition = godot::Vector3{
      static_cast<real_t>(position.x * Tile::LengthInCells * cellSize), 0,
      static_cast<real_t>(position.y * Tile::LengthInCells * cellSize)};
  auto sideSize =
      static_cast<real_t>(Tile::LengthInCells * MapGenerator::cellSize);
  auto size = godot::Vector3{sideSize, MapGenerator::cellSize / 4.0f, sideSize};
  return globalTilePosition + size / 2; // Shift from origin to the edge
}

auto MapGenerator::CreateEnemySpawner() -> EnemySpawner * {
  auto *spawner = memnew(game::EnemySpawner);
  spawner->add_to_group("Spawners");

  return spawner;
}

auto MapGenerator::AddEnemySpawners(Position position,
                                    DirectionsVector directions,
                                    godot::Ref<godot::AStar3D> aStar) -> void {
  DeleteNearestSpawner(calculateTilePosition3D(position)); // TODO think!
  if (directions.empty()) {
    AddEnemySpawnerForPortal(position, aStar);
  }

  for (auto direction : directions) {
    auto spawner = CreateEnemySpawner();
    add_child(spawner);

    auto position3d = CalculateEnemySpawnerPosition(position, direction);
    spawner->SetPosition(position3d);
    auto nearestWayPointId = aStar->get_closest_point(position3d);
    auto path = aStar->get_point_path(nearestWayPointId, 0);
    spawner->SetPath(path);
    spawnersCount += 1;
  }
}

auto MapGenerator::AddEnemySpawnerForPortal(
    Position position, godot::Ref<godot::AStar3D> aStar) -> void {
  auto spawner = CreateEnemySpawner();
  add_child(spawner);

  auto position3d = calculateTilePosition3D(position);
  spawner->SetPosition(position3d);
  auto nearestWayPointId = aStar->get_closest_point(position3d);
  const auto path = aStar->get_point_path(nearestWayPointId, 0);
  spawner->SetPath(path);
  spawnersCount += 1;
}

auto MapGenerator::CalculateEnemySpawnerPosition(
    Position position, Direction direction) -> godot::Vector3 {
  auto position3d = calculateTilePosition3D(position);
  auto positionShift = Tile::LengthInCells * cellSize / 2;
  switch (direction) {
  case Direction::Up:
    position3d.z -= positionShift;
    break;
  case Direction::Left:
    position3d.x -= positionShift;
    break;
  case Direction::Down:
    position3d.z += positionShift;
    break;
  case Direction::Right:
    position3d.x += positionShift;
    break;
  }
  return position3d;
}

auto MapGenerator::UpdateEnemySpawnersEnemies(const EnemiesVector &enemies)
    -> void { // FIXME remove copying, replace with spans
  auto enemiesCount = enemies.size();
  auto enemiesCountPerSpawner = enemiesCount / spawnersCount;
  auto it = enemies.cbegin();

  auto children = get_children();

  for (std::size_t i; i < children.size(); ++i) {
    std::size_t restEnemies = std::distance(it, enemies.end());
    EnemiesVector spawnerEnemies(it, it + std::min(enemiesCount, restEnemies));
    auto *node =
        dynamic_cast<Node *>(static_cast<godot::Object *>(children[i]));
    if (node && node->is_in_group("Spawners")) {
      auto spawner = static_cast<EnemySpawner *>(node);
      spawner->SetEnemies(spawnerEnemies);
    }
  }
}

auto MapGenerator::DeleteNearestSpawner(godot::Vector3 position) -> void {
  auto children = get_children();

  EnemySpawner *spawnerToDelete = nullptr;
  auto minDistance = std::numeric_limits<real_t>{}.max();

  for (std::size_t i; i < children.size(); ++i) {
    auto *node =
        dynamic_cast<Node *>(static_cast<godot::Object *>(children[i]));
    if (node && node->is_in_group("Spawners")) {
      auto spawner = static_cast<EnemySpawner *>(node);
      auto spawnerPosition = spawner->GetPosition();
      auto distanceToSpawner = position.distance_squared_to(spawnerPosition);
      if (distanceToSpawner < minDistance) {
        minDistance = distanceToSpawner;
        spawnerToDelete = spawner;
      }
    }
  }

  if (spawnerToDelete) {
    remove_child(spawnerToDelete);
    spawnerToDelete->queue_free();
  }
}

auto MapGenerator::AddWayPointFrom(Position position,
                                   Direction direction) -> void {
  auto position3d = calculateTilePosition3D(position);
  auto nearestWayPointId = aStar->get_closest_point(position3d);
  aStar->add_point(lastAStarId, position3d);
  aStar->connect_points(lastAStarId++, nearestWayPointId, false);
}

auto MapGenerator::AddWayPointTo(Position position,
                                 Direction direction) -> void {
  auto tileSideSize = Tile::LengthInCells * cellSize;
  auto position3d = calculateTilePosition3D(position);
  auto nearestWayPointId = aStar->get_closest_point(position3d);

  auto positionShift = tileSideSize / 2.0f;
  switch (direction) {
  case Direction::Up:
    position3d.z -= positionShift;
    break;
  case Direction::Left:
    position3d.x -= positionShift;
    break;
  case Direction::Down:
    position3d.z += positionShift;
    break;
  case Direction::Right:
    position3d.x += positionShift;
    break;
  }

  aStar->add_point(lastAStarId, position3d);
  aStar->connect_points(lastAStarId++, nearestWayPointId, false);
}
auto MapGenerator::AddWayPoints(Position position,
                                DirectionsVector directions) -> void {
  for (auto direction : directions) {
    AddWayPointTo(position, direction);
  }
}

// ------------------------------------------------------------------------

std::uint8_t MapGenerator::cellSize = 20;

auto MapGenerator::SetStartDirectionsCount(std::size_t startDirectionsCount)
    -> void {
  this->startDirectionsCount = startDirectionsCount;
}

auto MapGenerator::GetStartDirectionsCount() -> std::size_t {
  return startDirectionsCount;
}

void MapGenerator::Extend(Position position, Direction direction) {
  // check direction
  auto neighbor = Move(position, direction);
  bool isTileExist = map.IsTileExist(neighbor);
  if (isTileExist) {
    // throw std::runtime_error("Try place tile on existing tile");
    std::terminate();
  }

  auto freePositions = GetFreePositionsAround(neighbor);

  auto freeTilesCount = GetFreeTilesCount();
  auto roadsCount = std::min(freeTilesCount, freePositions.size());

  freePositions.resize(roadsCount);

  std::ranges::shuffle(freePositions, generator);

  Tile tile = GenerateTile(GetOppositeDirection(direction), freePositions);
  if (freePositions.empty()) {
    auto center = Position{Tile::LengthInCells / 2, Tile::LengthInCells / 2};
    tile.SetCell(center, PortalCell{});
  }
  map.SetTile(neighbor, std::move(tile));

  AddWayPointFrom(neighbor, GetOppositeDirection(direction));

  PrepairTileFor3D(neighbor);

  auto directions = GetExtendRoads(neighbor, GetOppositeDirection(direction));
  AddWayPoints(neighbor, directions);
  if (freePositions.size() >= 1 && freeTilesCount > 0) {
    AddExtendTiles(neighbor, directions);
  }

  // TODO: GetNewEnemies
  auto enemies = EnemiesVector(10, TestEnemy{});
  // TODO set path to spawner or to enemy
  AddEnemySpawners(neighbor, directions, aStar);
  UpdateEnemySpawnersEnemies(enemies);
}

void MapGenerator::AddStartTile() {
  Position center{0, 0};
  // TODO generate
  DirectionsVector allDirections{Direction::Up, Direction::Left,
                                 Direction::Down, Direction::Right};
  std::ranges::shuffle(allDirections, generator);
  auto directions_view =
      allDirections |
      std::ranges::views::drop(allDirections.size() - startDirectionsCount);

  auto directions =
      DirectionsVector(directions_view.begin(), directions_view.end());
  Tile tile = GenerateStartTile(directions);

  map.SetTile(center, std::move(tile));

  // tilesForExtension.emplace_back(center);
}

Tile MapGenerator::GenerateStartTile(DirectionsVector directions) {
  Tile tile{};

  for (std::int64_t i = 0; i < Tile::LengthInCells; ++i) {
    for (std::int64_t j = 0; j < Tile::LengthInCells; ++j) {
      std::uint8_t elevation = distribution(generator) % 4;
      GrassCell cell{elevation};
      tile.SetCell(Position{j, i}, std::move(cell));
    }
  }

  tile.SetCell(Position{Tile::LengthInCells / 2, Tile::LengthInCells / 2},
               BaseCell{});
  AddRoads(tile, directions);

  return tile;
}
Tile MapGenerator::GenerateTile(Direction extendFrom,
                                DirectionsVector possibleExtensions) {
  Tile tile{};
  for (std::int64_t i = 0; i < Tile::LengthInCells; ++i) {
    for (std::int64_t j = 0; j < Tile::LengthInCells; ++j) {
      std::uint8_t elevation = distribution(generator) % 4;
      GrassCell cell{elevation};
      tile.SetCell(Position{j, i}, std::move(cell));
    }
  }
  AddRoad(tile, extendFrom);
  if (!possibleExtensions.empty()) {
    auto roadDistribution =
        std::uniform_int_distribution<>(1, possibleExtensions.size());
    auto wayoutRoadsCount = roadDistribution(generator);

    auto directions_view =
        possibleExtensions |
        std::ranges::views::drop(possibleExtensions.size() - wayoutRoadsCount);

    AddRoads(tile,
             DirectionsVector(directions_view.begin(), directions_view.end()));
  }

  return tile;
}
void MapGenerator::AddRoad(Tile &tile, Direction direction) {
  auto IsBaseOrPortalCell = [](CellType c) {
    return std::holds_alternative<BaseCell>(c) ||
           std::holds_alternative<PortalCell>(c);
  };
  Position position{Tile::LengthInCells / 2, Tile::LengthInCells / 2};

  auto IsInsideTile = [](Position pos) {
    return pos.x >= 0 && pos.y >= 0 && pos.x < Tile::LengthInCells &&
           pos.y < Tile::LengthInCells;
  };

  while (IsInsideTile(position)) {
    auto isBaseOrPortalCell = IsBaseOrPortalCell(tile.GetCell(position));
    if (!isBaseOrPortalCell) {
      tile.SetCell(position, RoadCell{});
    }
    position = Move(position, direction);
  }
}

auto MapGenerator::AddRoads(Tile &tile, DirectionsVector directions) -> void {
  for (auto direction : directions) {
    AddRoad(tile, direction);
  }
}

bool MapGenerator::IsTileForExtention(Position position) {

  auto IsTileForExtentionFrom = [&](Position position, Direction direction) {
    auto neighbor = Move(position, direction);
    bool isTileExist = map.IsTileExist(neighbor);
    if (isTileExist) {
      auto tile = *map.GetTile(neighbor);
      auto hasRoad = HasRoad(neighbor, GetOppositeDirection(direction));
      if (hasRoad) {
        return true;
      }
    }
    return false;
  };

  std::array<bool, 4> results{};
  results[0] = IsTileForExtentionFrom(position, Direction::Up);
  results[1] = IsTileForExtentionFrom(position, Direction::Left);
  results[2] = IsTileForExtentionFrom(position, Direction::Down);
  results[3] = IsTileForExtentionFrom(position, Direction::Right);

  return std::ranges::any_of(results, [](bool b) { return b == true; });
}
DirectionsVector MapGenerator::GetFreePositionsAround(Position position) {
  DirectionsVector freePositions;
  Direction direction{};
  auto CheckNeighbor = [&](Position neighborPos) {
    bool isTileExist = map.IsTileExist(neighborPos);
    if (!isTileExist) {
      // check if neigbor are going to extend here
      bool isForExtention = IsTileForExtention(neighborPos);
      if (!isForExtention) {
        freePositions.emplace_back(direction);
      }
    }
  };

  direction = Direction::Up;
  auto neigborPos = Move(position, direction);
  CheckNeighbor(neigborPos);

  direction = Direction::Left;
  neigborPos = Move(position, direction);
  CheckNeighbor(neigborPos);

  direction = Direction::Down;
  neigborPos = Move(position, direction);
  CheckNeighbor(neigborPos);

  direction = Direction::Right;
  neigborPos = Move(position, direction);
  CheckNeighbor(neigborPos);

  return freePositions;
}

auto MapGenerator::GetFreeTilesCount() const -> std::size_t {
  auto freeTiles =
      static_cast<int64_t>(maxTilesCount - map.Size() - extendTilesCount);
  return std::clamp(freeTiles, std::int64_t{0},
                    static_cast<int64_t>(maxTilesCount));
}

bool MapGenerator::HasRoad(Position pos, Direction direction) {
  auto tile = map.GetTile(pos); // Should exist, I hope
  switch (direction) {
  case Direction::Up:
    return std::holds_alternative<RoadCell>(
        tile->GetCell(Position{Tile::LengthInCells / 2, 0}));
  case Direction::Left:
    return std::holds_alternative<RoadCell>(
        tile->GetCell(Position{0, Tile::LengthInCells / 2}));
  case Direction::Down:
    return std::holds_alternative<RoadCell>(tile->GetCell(
        Position{Tile::LengthInCells / 2, Tile::LengthInCells - 1}));
  case Direction::Right:
    return std::holds_alternative<RoadCell>(tile->GetCell(
        Position{Tile::LengthInCells - 1, Tile::LengthInCells / 2}));
  }
  return {};
}
std::size_t MapGenerator::RoadsCount(Position pos) {
  std::size_t count = 0;
  count += static_cast<std::size_t>(HasRoad(pos, Direction::Up));
  count += static_cast<std::size_t>(HasRoad(pos, Direction::Left));
  count += static_cast<std::size_t>(HasRoad(pos, Direction::Down));
  count += static_cast<std::size_t>(HasRoad(pos, Direction::Right));
  return count;
}
DirectionsVector MapGenerator::GetAllRoads(Position pos) {
  DirectionsVector roads;
  Direction direction{Direction::Up};
  if (HasRoad(pos, direction)) {
    roads.emplace_back(direction);
  }
  direction = Direction::Left;
  if (HasRoad(pos, direction)) {
    roads.emplace_back(direction);
  }
  direction = Direction::Down;
  if (HasRoad(pos, direction)) {
    roads.emplace_back(direction);
  }
  direction = Direction::Right;
  if (HasRoad(pos, direction)) {
    roads.emplace_back(direction);
  }

  return roads;
}

auto MapGenerator::GetExtendRoads(Position position,
                                  Direction extendFrom) -> DirectionsVector {
  auto allRoads = GetAllRoads(position);
  const auto ret = std::ranges::remove(allRoads, extendFrom);
  allRoads.erase(ret.begin(), ret.end());
  return allRoads;
}

Position MapGenerator::Move(Position pos, Direction direction) {
  switch (direction) {
  case Direction::Up:
    pos.y -= 1;
    break;
  case Direction::Left:
    pos.x -= 1;
    break;
  case Direction::Down:
    pos.y += 1;
    break;
  case Direction::Right:
    pos.x += 1;
    break;
  }
  return pos;
}

Direction MapGenerator::GetOppositeDirection(Direction direction) {
  switch (direction) {
  case Direction::Up:
    return Direction::Down;
  case Direction::Left:
    return Direction::Right;
  case Direction::Down:
    return Direction::Up;
  case Direction::Right:
    return Direction::Left;
  }
  return {};
}

// ExtendButton stuff -------------------------------------------------

// auto ExtendButton::_bind_methods() -> void {
//   ADD_SIGNAL(MethodInfo(
//       "extend_pressed",
//       PropertyInfo(godot::Variant::PACKED_INT64_ARRAY,
//       "extend_from_position"), PropertyInfo(Variant::INT, "direction")));
// }

// auto ExtendButton::_ready() -> void { set_text("Extend"); }

// auto ExtendButton::_process(double) -> void {
//   if (is_pressed()) {
//     auto array = godot::PackedInt64Array{};
//     array.append(extendFromPosition.x);
//     array.append(extendFromPosition.y);

//     emit_signal("extend_pressed", array, static_cast<int>(direction));
//     get_parent()->remove_child(this);
//     queue_free();
//   }
// }

// auto ExtendButton::SetExtendFromPosition(Position position) -> void {
//   extendFromPosition = position;
// }

// auto ExtendButton::SetDirection(Direction direction) -> void {
//   this->direction = direction;
// }

//------------------------------------------------------------

auto ExtendTile::_bind_methods() -> void {
  ADD_SIGNAL(MethodInfo(
      "extend_pressed",
      PropertyInfo(godot::Variant::PACKED_INT64_ARRAY, "extend_from_position"),
      PropertyInfo(Variant::INT, "direction")));

  godot::ClassDB::bind_method(godot::D_METHOD("interact"),
                              &ExtendTile::interact);
}

auto ExtendTile::_ready() -> void {

  auto sideSize =
      static_cast<real_t>(Tile::LengthInCells * MapGenerator::cellSize);
  auto size = godot::Vector3{sideSize, MapGenerator::cellSize / 4.0f, sideSize};
  auto color = godot::Color{0, 0, 1, 0.8};

  auto *staticBody = CreateBox(size, color);

  // auto *label = memnew(godot::Label3D);
  // label->set_text("Scan Area");

  // staticBody->add_child(label);

  // label->set_position(godot::Vector3{});
  // label->show();

  add_child(staticBody);
}

auto ExtendTile::_process(double) -> void {}

auto ExtendTile::interact() -> void {
  auto array = godot::PackedInt64Array{};
  array.append(extendFromPosition.x);
  array.append(extendFromPosition.y);

  [[maybe_unused]] auto error =
      emit_signal("extend_pressed", array, static_cast<int>(direction));

  get_parent()->remove_child(this);
  queue_free();
}

auto ExtendTile::SetExtendFromPosition(Position position) -> void {
  extendFromPosition = position;
}

auto ExtendTile::SetDirection(Direction direction) -> void {
  this->direction = direction;
}

} // namespace game