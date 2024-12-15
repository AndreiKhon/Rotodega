
#ifndef STATUSEFFECTSHPP
#define STATUSEFFECTSHPP

#include <cstdint>

namespace game {

struct Effect {
  std::int64_t value;
};

struct Slow : public Effect {
  constexpr static std::int64_t deplete = 6;
  static std::int64_t max;
};
inline std::int64_t Slow::max = 60;

struct Haste : public Effect {
  constexpr static std::int64_t deplete = 6;
  static std::int64_t max;
};
inline std::int64_t Haste::max = 60;

struct Bleed : public Effect { // TODO rename
  static std::int64_t deplete;
};
inline std::int64_t Bleed::deplete = 25;

struct Burn : public Effect {
  static std::int64_t deplete;
};
inline std::int64_t Burn::deplete = 25;

struct Poison : public Effect {
  static std::int64_t deplete;
};
inline std::int64_t Poison::deplete = 25;

} // namespace game

#endif