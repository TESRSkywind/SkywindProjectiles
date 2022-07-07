#pragma once

class FenixProjs
{
public:
	enum class Types : uint32_t
	{
		Normal,
		CustomPos
	};

	constexpr static uint32_t NormalType() {
		return static_cast<uint32_t>(Types::Normal);
	}

	constexpr static uint32_t CustomPosType()
	{
		return static_cast<uint32_t>(Types::CustomPos);
	}
};

bool is_CustomPosType(RE::Projectile* proj);
void set_CustomPosType(RE::Projectile* proj);
void set_NormalType(RE::Projectile* proj);
