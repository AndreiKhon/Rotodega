
#ifndef ENEMYHPP
#define ENEMYHPP

#include <variant>
#include <vector>

namespace game {

struct TestEnemy {};

using EnemyType = std::variant<TestEnemy>;

using EnemiesVector = std::vector<EnemyType>;

} // namespace game

#endif