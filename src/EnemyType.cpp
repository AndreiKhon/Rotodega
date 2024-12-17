
#include "EnemyType.hpp"
#include "StatusEffect.hpp"
#include <variant>

namespace game {

EnemyType::EnemyType(EnemyVariant enemy) : enemy(enemy) {}

struct GetEnemyHPVisitor {
  auto operator()(BasicEnemy auto enemy) -> HitPoints { return enemy.hp; }
};
auto EnemyType::GetHP() -> HitPoints const {
  auto hp = std::visit(GetEnemyHPVisitor{}, enemy);
  return hp;
}

struct GetEnemySpeedVisitor {
  auto operator()(BasicEnemy auto enemy) -> double { return enemy.speed; }
};
auto EnemyType::GetSpeed() -> double const {
  auto speed = std::visit(GetEnemySpeedVisitor{}, enemy);
  return speed;
}

struct SetEnemySpeedVisitor {
  auto operator()(BasicEnemy auto enemy,
                  std::variant<double> speed) -> EnemyVariant {
    enemy.speed = std::get<0>(speed);
    return enemy;
  }
};
auto EnemyType::SetSpeed(double speed) -> void {
  enemy =
      std::visit(SetEnemySpeedVisitor{}, enemy, std::variant<double>(speed));
}

struct ApplyEffectOnEnemy {
  auto operator()(BasicEnemy auto enemy, Slow effect) -> EnemyVariant {
    enemy.speed *= (100.0 - effect.value) / 100.0;
    return enemy;
  }
  auto operator()(BasicEnemy auto enemy, Haste effect) -> EnemyVariant {
    enemy.speed *= (100.0 + effect.value) / 100;
    return enemy;
  }
  auto operator()(BasicEnemy auto enemy, Bleed effect) -> EnemyVariant {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete;
      return enemy;
    }
    return {};
  }
  auto operator()(BasicEnemy auto enemy, Burn effect) -> EnemyVariant {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete / 2;
      return enemy;
    }
    return {};
  }
  auto operator()(BasicEnemy auto enemy, Poison effect) -> EnemyVariant {
    if (enemy.shield > 0) {
      enemy.shield -= effect.deplete;
      return enemy;
    }
    if (enemy.armor > 0) {
      enemy.armor -= effect.deplete / 2;
      return enemy;
    }
    if (enemy.health > 0) {
      enemy.health -= effect.deplete / 2;
      return enemy;
    }
    return {};
  }
};
auto EnemyType::ApplyEffect(EffectType effect) -> void {
  enemy = std::visit(ApplyEffectOnEnemy{}, enemy, effect);
}

} // namespace game