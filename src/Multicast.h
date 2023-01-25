#pragma once

#include "JsonUtils.h"

namespace ManyProjs
{
	// Before read json ingame
	void forget();

	// Read json
	void add_mod(const Json::Value& homing, int hex);

	// Apply hooks
	void install();

	namespace Casting
	{
		struct CastData
		{
			RE::Actor* caster;
			ProjectileRot parallel_rot;
			RE::SpellItem* spel;
			RE::NiPoint3* startPos;
		};

		/// Perform multicast. <param name="key">used for find multicast data</param>
		void onManyCasted(uint32_t key, CastData& cast_data);
	}
}
