#pragma once
#include "FenixProjectilesAPI.h"

void read_json();
void reset_json();

RE::ProjectileHandle cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel,
	const RE::NiPoint3& start_pos, const ProjectileRot& rot, bool withSound = false);

RE::ProjectileHandle cast_CustomPos(RE::Actor* caster, RE::TESAmmo* ammo, RE::TESObjectWEAP* weap,
	const RE::NiPoint3& start_pos, const ProjectileRot& rot, bool withSound = false);

bool is_MyBeamType(RE::Projectile* proj);

namespace Sounds
{
	void play_cast_sound(RE::TESObjectREFR* caster, RE::SpellItem* spel,
		const RE::NiPoint3& start_pos);
}
