
#ifndef MAPHPP
#define MAPHPP

#include "Tile.hpp"

#include <cstddef>
#include <godot_cpp/classes/node.hpp>
#include <optional>
#include <unordered_map>

using namespace godot;

namespace game {

auto operator==(const Position &lhs, const Position &rhs) -> bool;
auto operator!=(const Position &lhs, const Position &rhs) -> bool;
struct PositionHash { // Not good for matrix coordinates
  auto operator()(const Position &p) const -> std::size_t {
    auto hash1 = std::hash<std::size_t>{}(p.x);
    auto hash2 = std::hash<std::size_t>{}(p.y);

    return hash1 ^ hash2;
  };
};

class Map {
  std::unordered_map<Position, Tile, PositionHash> map;

public:
  auto GetTile(Position pos) -> std::optional<Tile>;
  auto SetTile(Position pos, Tile &&tile) -> void;
  bool IsTileExist(Position pos) const;
  auto Size() const -> std::size_t;
};

} // namespace game

#endif