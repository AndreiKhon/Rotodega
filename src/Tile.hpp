
#ifndef TILEHPP
#define TILEHPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

namespace game {

enum class Direction { Up, Left, Down, Right };
using DirectionsVector =
    std::vector<Direction>; // Should be replaced with
                            // std::array<std::optionaL<Direction>, 4>?

struct Position {
  std::int64_t x;
  std::int64_t y;
};

struct GrassCell {
  std::size_t elevation = 1;
};
struct RoadCell {};
struct BaseCell {
  std::size_t health = 15;
};
struct PortalCell {};
using CellType = std::variant<GrassCell, RoadCell, BaseCell, PortalCell>;

class Tile {
public:
  static constexpr std::uint8_t LengthInCells = 7;

private:
  std::array<CellType, LengthInCells * LengthInCells> cells;

public:
  void SetCell(Position pos, CellType cell);
  CellType GetCell(Position pos);

private:
  std::size_t GetCellIndex(Position pos);
};

} // namespace game

#endif