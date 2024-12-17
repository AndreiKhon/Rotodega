
#include "StatusEffect.hpp"
#include "godot_cpp/classes/timer.hpp"

namespace game {

auto StatusEffect::_bind_methods() -> void {
  ADD_SIGNAL(godot::MethodInfo("effect_depleted",
                               godot::PropertyInfo(godot::Variant::INT, "id")));
  ADD_SIGNAL(godot::MethodInfo("effect_updated",
                               godot::PropertyInfo(godot::Variant::INT, "id")));

  godot::ClassDB::bind_method(godot::D_METHOD("on_timer"),
                              &StatusEffect::on_timer);
}

auto StatusEffect::_ready() -> void {
  auto *timer = memnew(godot::Timer);
  timer->set_wait_time(1);
  timer->connect("timeout", godot::Callable(this, "on_timer"));
  add_child(timer);
  timer->start();
}

auto StatusEffect::on_timer() -> void {
  effect = std::visit(DepleteEffectVisitor{}, effect);
  auto remain = std::visit(EffectValueVisitor{}, effect);
  auto id = get_instance_id();
  if (remain <= 0) {
    emit_signal("effect_depleted", id);
  } else {
    emit_signal("effect_updated", id);
  }
}

StatusEffect::StatusEffect(EffectType effect) : effect(effect) {}

auto StatusEffect::Update(EffectType effect) -> void {
  this->effect = std::visit(UpdateEffectVisitor{}, this->effect, effect);
}

auto StatusEffect::GetType() -> EffectType const { return effect; }

} // namespace game