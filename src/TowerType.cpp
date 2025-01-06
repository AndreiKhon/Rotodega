
#include "TowerType.hpp"
#include "StatusEffect.hpp"
#include "Towers.hpp"
#include <variant>

namespace game {

TowerType::TowerType(TowerVariant tower) : tower(tower) {}

auto TowerType::AddEffect(EffectType effect) -> void {
  tower = std::visit(
      [](BasicTower auto tower, EffectType effect) -> TowerVariant {
        tower.effects.push_back(effect);
        return tower;
      },
      tower, (effect));
}

auto TowerType::GetBaseDamage() -> std::uint64_t const {
  auto baseDamage = std::visit(
      [](BasicTower auto tower) -> std::uint64_t { return tower.baseDamage; },
      tower);
  return baseDamage;
}

auto TowerType::GetMultipliers() -> HPMultiplier const {
  auto multipliers = std::visit(
      [](BasicTower auto tower) { return tower.multipliers; }, tower);
  return multipliers;
}

auto TowerType::GetRange() -> double const {
  auto range = std::visit(
      [](BasicTower auto tower) -> double { return tower.range; }, tower);
  return range;
}

auto TowerType::GetRPM() -> std::uint64_t const {
  auto rpm = std::visit(
      [](BasicTower auto tower) -> std::uint64_t { return tower.rpm; }, tower);
  return rpm;
}

auto TowerType::GetElectricity() -> std::uint64_t const {
  auto electricity = std::visit(
      [](BasicTower auto tower) -> std::uint64_t { return tower.electricity; },
      tower);
  return electricity;
}

auto TowerType::GetEffects() -> EffectVector const {
  auto effects = std::visit(
      [](BasicTower auto tower) -> EffectVector { return tower.effects; },
      tower);
  return effects;
}

auto TowerType::GetCrits() -> CriticalChance const {
  auto crits = std::visit(
      [](BasicTower auto tower) -> CriticalChance { return tower.crits; },
      tower);
  return crits;
}

struct GetTowerModelPathVisitor {
  auto operator()(BasicTower auto tower) -> const char * {
    return "res://test_enemy.glb";
  }
};

auto TowerType::GetModelPath() -> const char *const {
  auto path = std::visit(GetTowerModelPathVisitor{}, tower);
  return path;
}

} // namespace game