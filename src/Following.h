#pragma once

#include "JsonUtils.h"

enum class FollowTypes : uint32_t
{
	Nimbus,
	Shield,
	Round
};

namespace Following
{
	// init, if needed, runtime data for following projs
	void onCreated(RE::Projectile* proj);

	// Set `proj` following type with data from `key`. Usually key is formid
	void onCreated(RE::Projectile* proj, uint32_t key, int ind = -1);

	// Before read json ingame
	void forget();

	// Read json
	void add_mod(const Json::Value& homing, int hex);

	// Apply hooks
	void install();
}
