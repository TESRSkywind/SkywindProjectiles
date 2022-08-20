#include "NewProjectiles.h"
#include "FenixProjectilesAPI.h"

constexpr FenixProjsType intToType(uint32_t type) { return *(FenixProjsType*)&type; }

bool is_CustomPosType(RE::Projectile* proj) { return intToType(proj->pad164).any(FenixProjsTypes::CustomPos); }

void set_CustomPosType(RE::Projectile* proj)
{
	proj->pad164 = intToType(proj->pad164).set(FenixProjsTypes::CustomPos).underlying();
}

bool is_AutoAimType(RE::Projectile* proj) { return intToType(proj->pad164).any(FenixProjsTypes::AutoAim); }

void set_AutoAimType(RE::Projectile* proj) {
	proj->pad164 = intToType(proj->pad164).set(FenixProjsTypes::AutoAim).underlying();
	proj->flags |= 1 << 6;  // smth like "no gravity". To updating model rotation.
}

void set_NormalType(RE::Projectile* proj) { proj->pad164 = 0; }

struct Projectile__LaunchData
{
	void* vftable_LaunchData_0;
	RE::NiPoint3 startPos;
	RE::NiPoint3 Point_14;
	RE::BGSProjectile* projectile;
	RE::TESObjectREFR* source;
	RE::CombatController* combatController;
	RE::TESObjectWEAP* weap;
	RE::TESAmmo* overwriteAmmo;
	float rotationZ;
	float rotationX;
	void* field_50;
	RE::TESObjectREFR* target;
	std::pair<float, float> drawn_time;
	RE::TESObjectCELL* cell;
	RE::MagicItem* CastItem;
	RE::MagicSystem::CastingSource castingSource;
	RE::EnchantmentItem* ammoEnchantment;
	RE::AlchemyItem* poison;
	uint32_t area;
	float field_94;
	float scale;
	char field_9C;
	char field_9D;
	char field_9E;
	char field_9F;
	char field_A0;
	char field_A1;
	char field_A2;
	char field_A3;
	char gapA4[4];
};
static_assert(sizeof(Projectile__LaunchData) == 0xA8);

void init_Launchdata(Projectile__LaunchData& ldata, RE::Actor* a, const RE::NiPoint3& startPos, RE::SpellItem* spel,
	float rotationZ, float rotationX, uint32_t area)
{
	auto mgef = FenixUtils::getAVEffectSetting(spel);

	ldata.vftable_LaunchData_0 = nullptr;  // TODO: mb used
	ldata.startPos = startPos;
	ldata.Point_14 = { 0.0f, 0.0f, 0.0f };
	ldata.projectile = mgef->data.projectileBase;
	ldata.source = a;
	ldata.combatController = a->combatController;
	ldata.weap = nullptr;
	ldata.overwriteAmmo = nullptr;
	ldata.rotationZ = rotationZ;
	ldata.rotationX = rotationX;
	ldata.field_50 = nullptr;
	ldata.target = nullptr;
	ldata.drawn_time.first = 0.0f;
	ldata.drawn_time.second = 0.0f;
	ldata.cell = a->GetParentCell();
	ldata.CastItem = spel;
	ldata.castingSource = RE::MagicSystem::CastingSource::kOther;
	ldata.ammoEnchantment = nullptr;
	ldata.poison = nullptr;
	ldata.area = area;
	ldata.field_94 = 1.0f;
	ldata.scale = 1.0f;
	*(uint32_t*)&ldata.field_9C = 0x10000;
	ldata.field_A0 = 0;
	ldata.field_A1 = 0;
	ldata.field_A2 = 0;
}

uint32_t* CreateProjectile_14074B170(uint32_t* a1, Projectile__LaunchData* LaunchData)
{
	return _generic_foo_<42928, decltype(CreateProjectile_14074B170)>::eval(a1, LaunchData);
}

RE::Effect* GetCostiestEffect(RE::MagicItem* spel, RE::MagicSystem::Delivery type, char ignorearea)
{
	return _generic_foo_<11216, decltype(GetCostiestEffect)>::eval(spel, type, ignorearea);
}

uint32_t cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	Projectile__LaunchData ldata;
	auto eff = GetCostiestEffect(spel, RE::MagicSystem::Delivery::kAimed, false);

	init_Launchdata(ldata, caster, start_pos, spel, rot.z, rot.x, eff ? eff->GetArea() : 0);

	uint32_t handle;
	CreateProjectile_14074B170(&handle, &ldata);

	return handle;
}

uint32_t cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	auto handle = cast(caster, spel, start_pos, rot);

	RE::TESObjectREFRPtr _refr;
	RE::LookupReferenceByHandle(handle, _refr);
	if (auto proj = _refr.get() ? _refr.get()->As<RE::Projectile>() : nullptr) {
		set_CustomPosType(proj);
	}

	return handle;
}

namespace Sounds
{

	RE::BGSSoundDescriptorForm* EffectSetting__get_sndr(RE::EffectSetting* a1, RE::MagicSystem::SoundID sid)
	{
		return _generic_foo_<11001, decltype(EffectSetting__get_sndr)>::eval(a1, sid);
	}

	void PlaySound_func3_140BEDB10(RE::BSSoundHandle* a1, RE::NiAVObject* source_node)
	{
		return _generic_foo_<66375, decltype(PlaySound_func3_140BEDB10)>::eval(a1, source_node);
	}

	char set_sound_position(RE::BSSoundHandle* shandle, float x, float y, float z)
	{
		return _generic_foo_<66370, decltype(set_sound_position)>::eval(shandle, x, y, z);
	}

	void play_cast_sound(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos)
	{
		if (auto root = caster->Get3D2()) {
			RE::BSSoundHandle shandle;
			RE::BGSSoundDescriptorForm* sndr = nullptr;
			auto sid = RE::MagicSystem::SoundID::kRelease;
			if (auto eff = FenixUtils::getAVEffectSetting(spel)) {
				sndr = EffectSetting__get_sndr(eff, sid);
			}
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(shandle, sndr, 0);
			PlaySound_func3_140BEDB10(&shandle, root);

			set_sound_position(&shandle, start_pos.x, start_pos.y, start_pos.z);
			shandle.Play();
		}
	}
}

uint32_t cast_CustomPos_withsound(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	auto handle = cast_CustomPos(caster, spel, start_pos, rot);

	Sounds::play_cast_sound(caster, spel, start_pos);

	return handle;
}

namespace AutoAim
{
	RE::Actor* findTarget(RE::Projectile* proj)
	{
		auto target = proj->desiredTarget.get().get();
		if (target)
			return target->As<RE::Actor>();

		auto caster = proj->shooter.get().get();
		if (!caster)
			return nullptr;

		float mindist2 = 1.0E15f;
		RE::TESObjectREFR* refr = nullptr;
		RE::TES::GetSingleton()->ForEachReference([=, &mindist2, &refr](RE::TESObjectREFR& _refr) {
			if (!_refr.IsDisabled() && !_refr.IsDead() && _refr.GetFormType() == RE::FormType::ActorCharacter &&
				_refr.formID != caster->formID) {
				float curdist = proj->GetPosition().GetSquaredDistance(_refr.GetPosition());
				if (curdist < mindist2) {
					mindist2 = curdist;
					refr = &_refr;
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});

		if (!refr || mindist2 > proj->range * proj->range)
			return nullptr;

#ifdef DEBUG
		char sMsgBuff[128] = { 0 };
		sprintf_s(sMsgBuff, "Target found: %s", refr->GetName());
		RE::DebugNotification(sMsgBuff);
#endif  // DEBUG

		proj->desiredTarget = refr->GetHandle();
		return refr->As<RE::Actor>();
	}

	RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime) {
		if (target->IsDead() || target->IsBleedingOut()) {
			return target->GetPosition();
		} else {
			RE::NiPoint3 ans;
			_generic_foo_<46045, void(RE::Actor * a, float time, RE::NiPoint3* ans)>::eval(target, dtime, &ans);
			return ans + RE::NiPoint3{ 0, 0, target->GetHeight() * 0.5f };
		}
	}

	void change_direction(RE::Projectile* proj, RE::NiPoint3*, [[maybe_unused]] float dtime) {
		if (auto target = findTarget(proj)) {
			const float MIN_DIST = 200.0f, MIN_VEL = 500.0f;
			const auto B = get_victim_pos(target, 0.5f);
			auto BA = B - proj->GetPosition();
			float BA_len = BA.Length();
			float V_len = proj->linearVelocity.Length();

			float speed = proj->GetBaseObject()->As<RE::BGSProjectile>()->data.speed * proj->GetPowerSpeedMult() * proj->unk18C;
			float t = proj->lifeRemaining / BA_len * speed * dtime;
			
			if (t > 1 || BA_len < MIN_DIST && (proj->linearVelocity.Dot(BA) > 0.9f * BA_len * V_len)) {
				const auto P = get_victim_pos(target, dtime);
				auto PA = P - proj->GetPosition();
				float PA_len = PA.Length();
				proj->linearVelocity = PA / PA_len * std::max(V_len, MIN_VEL);
				//draw_line(proj->GetPosition(), proj->GetPosition() + proj->linearVelocity);
			} else {
				// A = Start, B = Finish, C = initial vel.
				// P_1 = A + t (C - A)
				// P_2 = C + t (B - C)
				// P   = P_1 + t (P_2 - P_1)
				//     = A + 2 t (C - A) + t^2 (A + B - 2 C)
				// P'  = 2 (C - A) + 2 t (A + B - 2 C)
				// ------------------------
				// P_1 = P - P' t / 2
				// P_2 = P_1 + (P - P_1) / t
				// C   = (P_2 - t B) / (1 - t)
				// A   = (P_1 - t C) / (1 - t)
				const auto& P = proj->data.location;
				const auto P_1 = P - proj->linearVelocity * t / 2;
				const auto P_2 = P_1 + (P - P_1) / t;
				const auto C = (P_2 - B * t) / (1 - t);
				const auto X = (P_1 - C * t) / (1 - t) + B - C * 2;
				proj->linearVelocity += X * 2 * dtime;
			}

			/*
			auto V = target->GetPosition() - proj->GetPosition();
			auto p = RE::PlayerCharacter::GetSingleton();
			auto speed = proj->linearVelocity.Length();
			V.Unitize();
			V *= speed;
			V -= proj->linearVelocity;
			V.Unitize();
			V *= p->GetActorValue(RE::ActorValue::kStamina);
			//float len = U.Cross(V).Unitize();
			speed = 2500.0f / 2.0f;
			proj->linearVelocity += V;
			float newspeed = proj->linearVelocity.Length();
			proj->linearVelocity *= speed / newspeed;
			*/

			_generic_foo_<43010, void(RE::TESObjectREFR * a, const RE::NiPoint3& P)>::eval(proj,
				proj->GetPosition() + proj->linearVelocity);
			
		} else {
			set_NormalType(proj);
		}
	}
}

extern "C" 
{
	DLLEXPORT void FenixProjectilesAPI_set_NormalType(RE::Projectile* proj) { return set_NormalType(proj); }

	DLLEXPORT void FenixProjectilesAPI_set_CustomPosType(RE::Projectile* proj) { return set_CustomPosType(proj); }

	DLLEXPORT void FenixProjectilesAPI_set_AutoAimType(RE::Projectile* proj) { return set_AutoAimType(proj); }

	DLLEXPORT uint32_t FenixProjectilesAPI_cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast(caster, spel, start_pos, rot);
	}

	DLLEXPORT uint32_t FenixProjectilesAPI_cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast_CustomPos(caster, spel, start_pos, rot);
	}

	DLLEXPORT uint32_t FenixProjectilesAPI_cast_CustomPos_withsound(RE::Actor* caster, RE::SpellItem* spel,
		const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast_CustomPos_withsound(caster, spel, start_pos, rot);
	}
}
