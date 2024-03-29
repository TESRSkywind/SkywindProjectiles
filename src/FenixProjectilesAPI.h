#pragma once

#include "json/json.h"

// Contains keys of new types. 0 is unset, -1 is clear.
struct NewProjType
{
	uint32_t emitter;
	uint32_t homing;
	uint32_t follower;

	void init(const Json::Value& item);
};
static_assert(sizeof(NewProjType) == 0xC);

using ProjectileRot = RE::Projectile::ProjectileRot;

typedef void (*set_type_t)(RE::Projectile* proj);

typedef uint32_t (*cast_t)(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot);
typedef uint32_t (
	*cast_CustomPos_t)(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot);
