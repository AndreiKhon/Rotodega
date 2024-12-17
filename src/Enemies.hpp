
#ifndef ENEMIESHPP
#define ENEMIESHPP

#include <cstdint>

namespace game {

struct HitPoints {
  std::uint64_t health;
  std::uint64_t armor;
  std::uint64_t shield;
};

struct TestEnemy0 {
  HitPoints hp{.health = 100, .armor = 0, .shield = 0};

  std::uint64_t damage = 1;
  std::uint64_t scraps = 5;

  double speed = 25;
};

struct TestEnemy1 {
  HitPoints hp{.health = 200, .armor = 50, .shield = 0};

  std::uint64_t damage = 1;
  std::uint64_t scraps = 10;

  double speed = 20;
};

struct TestEnemy2 {
  HitPoints hp{.health = 300, .armor = 150, .shield = 50};

  std::uint64_t damage = 1;
  std::uint64_t scraps = 15;

  double speed = 50;
};

} // namespace game

#endif