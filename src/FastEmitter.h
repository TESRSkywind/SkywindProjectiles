#pragma once

#include "JsonUtils.h"

namespace FastEmitters
{
	// in RuntimeData, 2 bits
	enum class Types : uint32_t
	{
		None,
		AccelerateToMaxSpeed,  // add some (hardcoded) value to speed
		Unused2,
		Unused3,

		Total
	};
	static_assert((uint32_t)Types::Total == 0x4);

	void onCreated(RE::Projectile* proj, Types type);

	// Apply hooks
	void install();
}
