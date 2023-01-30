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
			RE::NiPoint3* startPos;
			
			struct SpellData
			{
				RE::SpellItem* spel;
			};

			struct ArrowData
			{
				RE::TESObjectWEAP* weap;
				RE::TESAmmo* ammo;
			};

			std::variant<SpellData, ArrowData> data;
		};

		bool is_ManyCasted(uint32_t key);

		/// Perform multicast. <param name="key">used for find multicast data</param>. cast_data.data is empty
		void onManyCasted(uint32_t key, CastData& cast_data);
	}
}
