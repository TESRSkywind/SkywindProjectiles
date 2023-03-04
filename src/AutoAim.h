#pragma once

#include "JsonUtils.h"

// Stored in RuntimeData
enum class AutoAimTypes : uint32_t
{
	ConstSpeed,
	ConstAccel
};
static constexpr AutoAimTypes AutoAimTypes__DEFAULT = AutoAimTypes::ConstSpeed;

namespace AutoAim
{
	// init, if needed, runtime data for autoaim projs
	void onCreated(RE::Projectile* proj);

	// Set `proj` autoaim type with data from `key`. Usually key is formid
	void onCreated(RE::Projectile* proj, uint32_t key);

	// Before read json ingame
	void forget();

	// Read json
	void add_mod(const Json::Value& homing, int hex);

	// Apply hooks
	void install();

	namespace Data
	{
		void disable_AutoAim(RE::Projectile* proj);
	}
}
