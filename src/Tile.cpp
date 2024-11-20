
#include "Tile.hpp"
#include <exception>

namespace game {

void Tile::SetCell(Position pos, CellType cell) {
  cells[GetCellIndex(pos)] = cell;
}

CellType Tile::GetCell(Position pos) { return cells[GetCellIndex(pos)]; }

std::size_t Tile::GetCellIndex(Position pos) {
  if (pos.x >= LengthInCells || pos.y >= LengthInCells) {
    // throw std::runtime_error("Wrong Cell Position");
    std::terminate();
  }
  return pos.y * LengthInCells + pos.x;
}

} // namespace game