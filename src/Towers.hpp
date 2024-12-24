
#ifndef TOWERSHPP
#define TOWERSHPP

#include "StatusEffect.hpp"
#include <cstdint>
namespace game {

struct HPMultiplier {
  std::uint64_t health;
  std::uint64_t armor;
  std::uint64_t shield;
};

struct CriticalChance {
  double x2 = 0.0;
  double x3 = 0.0;
  double x4 = 0.0;
};

struct TestTower0 {
  std::uint64_t baseDamage = 15;
  HPMultiplier multipliers{.health = 15, .armor = 10, .shield = 5};
  double range = 100;
  std::uint64_t electricity = 0;
  static std::uint64_t rpm;
  static EffectVector effects;
  static CriticalChance crits;
};
inline std::uint64_t TestTower0::rpm = 15;
inline EffectVector TestTower0::effects{};
inline CriticalChance TestTower0::crits{};

struct TestTower1 {
  std::uint64_t baseDamage = 12;
  HPMultiplier multipliers{.health = 5, .armor = 10, .shield = 8};
  double range = 150;
  std::uint64_t electricity = 0;
  static std::uint64_t rpm;
  static EffectVector effects;
  static CriticalChance crits;
};
inline std::uint64_t TestTower1::rpm = 5;
inline EffectVector TestTower1::effects{};
inline CriticalChance TestTower1::crits{};

} // namespace game

#endif