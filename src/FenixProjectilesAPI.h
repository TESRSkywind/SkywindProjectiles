#pragma once

struct ProjectileRot
{
	float x, z;
};

typedef uint32_t (*cast_t)(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot);
typedef uint32_t (
	*cast_CustomPos_t)(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot);

