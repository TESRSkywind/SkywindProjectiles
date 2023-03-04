#include "Targeting.h"

namespace Targeting
{
	namespace Data
	{
		AimTargetType read_json_entry(const Json::Value& item)
		{
			return parse_enum_ifIsMember<AimTargetType__DEFAULT>(item, "target"sv);
		}

		uint32_t read_detectionAngle(const Json::Value& item, AimTargetType target)
		{
			return detection_angle_t::f2u(target == AimTargetType::Cursor ? item["cursorAngle"].asFloat() : 0.0f);
		}

		float get_detectionAngle(uint32_t detection_angle) { return detection_angle_t::u2f(detection_angle); }
	}

	RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime)
	{
		RE::NiPoint3 ans, eye_pos;
		target->GetLinearVelocity(ans);
		ans *= dtime;
		FenixUtils::Actor__get_eye_pos(target, eye_pos, 3);
		ans += eye_pos;
		return ans;
	}

	namespace Cursor
	{
		static bool is_anglebetween_less(const RE::NiPoint3& A, const RE::NiPoint3& B1, const RE::NiPoint3& B2, float angle_deg)
		{
			auto AB1 = B1 - A;
			auto AB2 = B2 - A;
			AB1.Unitize();
			AB2.Unitize();
			return acos(AB1.Dot(AB2)) < angle_deg / 180.0f * 3.1415926f;
		}

		static bool is_near_to_cursor(RE::Actor* caster, RE::Actor* target, float angle)
		{
			RE::NiPoint3 caster_pos, caster_sight, target_pos;

			FenixUtils::Actor__get_eye_pos(caster, caster_pos, 2);
			FenixUtils::Actor__get_eye_pos(target, target_pos, 3);

			caster_sight = caster_pos;
			caster_sight += FenixUtils::rotate(1, caster->data.angle);

			return is_anglebetween_less(caster_pos, caster_sight, target_pos, angle);
		}

		enum class LineOfSightLocation : std::uint32_t
		{
			kNone = 0,
			kEyes = 1,   // Eye level
			kHead = 2,   // 85%
			kTorso = 3,  // 50%
			kFeet = 4    // 15%
		};
		static_assert(sizeof(LineOfSightLocation) == 0x4);

		static LineOfSightLocation IsActorInLineOfSight(RE::Actor* caster, RE::Actor* target, float viewCone = 100)
		{
			return _generic_foo_<36752, decltype(IsActorInLineOfSight)>::eval(caster, target, viewCone);
		}

		RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle)
		{
			if (!_caster->IsPlayerRef())
				return nullptr;

			auto caster = _caster->As<RE::Actor>();
			std::vector<std::pair<RE::Actor*, float>> targets;

			RE::TES::GetSingleton()->ForEachReference([caster, &targets, angle](RE::TESObjectREFR& _refr) {
				if (auto refr = _refr.As<RE::Actor>(); refr && !_refr.IsDisabled() && !_refr.IsDead() &&
													   _refr.GetFormType() == RE::FormType::ActorCharacter &&
													   _refr.formID != caster->formID && is_near_to_cursor(caster, refr, angle)) {
					if (IsActorInLineOfSight(caster, refr) != LineOfSightLocation::kNone) {
						targets.push_back({ refr, caster->GetPosition().GetSquaredDistance(refr->GetPosition()) });
					}
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});

			if (!targets.size())
				return nullptr;

			return (*std::min_element(targets.begin(), targets.end(),
						[](const std::pair<RE::Actor*, float>& a, const std::pair<RE::Actor*, float>& b) {
							return a.second < b.second;
						}))
			    .first;
		}
	}

	bool is_hostile(RE::TESObjectREFR* refr, RE::TESObjectREFR* _caster)
	{
		auto target = refr->As<RE::Actor>();
		auto caster = _caster->As<RE::Actor>();
		if (!target || !caster)
			return false;

		return target->currentCombatTarget.get().get() == caster;
	}

	RE::TESObjectREFR* find_nearest_target(RE::TESObjectREFR* caster, RE::TESObjectREFR* origin, bool onlyHostile)
	{
		float mindist2 = 1.0E15f;
		RE::TESObjectREFR* refr = nullptr;
		RE::TES::GetSingleton()->ForEachReference([=, &mindist2, &refr](RE::TESObjectREFR& _refr) {
			if (!_refr.IsDisabled() && !_refr.IsDead() && _refr.GetFormType() == RE::FormType::ActorCharacter &&
				_refr.formID != caster->formID && (!onlyHostile || is_hostile(&_refr, caster))) {
				float curdist = origin->GetPosition().GetSquaredDistance(_refr.GetPosition());
				if (curdist < mindist2) {
					mindist2 = curdist;
					refr = &_refr;
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});

		if (!refr)
			return nullptr;

		return refr;
	}

	RE::Actor* findTarget(RE::TESObjectREFR* origin, const std::tuple<AimTargetType, float>& target_data)
	{
		auto proj = origin->As<RE::Projectile>();

		if (proj) {
			auto target = proj->desiredTarget.get().get();
			if (target)
				return target->As<RE::Actor>();
		}

		auto caster = proj ? proj->shooter.get().get() : origin;
		if (!caster)
			return nullptr;

		if (auto caster_npc = caster->As<RE::Actor>(); caster_npc && !caster_npc->IsPlayerRef()) {
			return caster_npc->currentCombatTarget.get().get();
		}

		RE::TESObjectREFR* refr;
		auto target_type = std::get<AimTargetType>(target_data);
		switch (target_type) {
		case AimTargetType::Nearest:
		case AimTargetType::Hostile:
			refr = find_nearest_target(caster, proj ? proj : caster, target_type == AimTargetType::Hostile);
			break;
		case AimTargetType::Cursor:
			refr = Cursor::find_cursor_target(caster, std::get<float>(target_data));
			break;
		default:
			refr = nullptr;
			break;
		}

		if (!refr)
			return nullptr;

#ifdef DEBUG
		FenixUtils::notification("Target found: %s", refr->GetName());
#endif  // DEBUG

		if (proj)
			proj->desiredTarget = refr->GetHandle();
		return refr->As<RE::Actor>();
	}
}
