#pragma once

#include "JsonUtils.h"

namespace Emitters
{
	// Before read json ingame
	void forget();

	// Read json mod
	void add_mod(const Json::Value& homing, int hex);

	// Read json global map
	void init_global(const Json::Value& emitters);

	// Apply hooks
	void install();

	void onCreated(RE::Projectile* proj, uint32_t big_key);
	void onCreated(RE::Projectile* proj);
}
