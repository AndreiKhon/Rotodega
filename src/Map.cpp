
#include "Map.hpp"
#include <cstddef>

namespace game {

auto Map::GetTile(Position pos) -> std::optional<Tile> {
  bool isTileExist = IsTileExist(pos);
  if (isTileExist) {
    const auto tile = map[pos];
    return std::optional<Tile>(tile);
  }
  return std::nullopt;
}
auto Map::SetTile(Position pos, Tile &&tile) -> void {
  map.emplace(std::make_pair(pos, tile));
}

auto Map::IsTileExist(Position pos) const -> bool {
  return map.find(pos) != map.end();
}

auto Map::Size() const -> std::size_t { return map.size(); }

auto operator==(const Position &lhs, const Position &rhs) -> bool {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

auto operator!=(const Position &lhs, const Position &rhs) -> bool {
  return !(lhs == rhs);
}

} // namespace game