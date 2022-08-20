#pragma once

class FenixProjs
{
public:
	enum class Types : uint32_t
	{
		Normal = 0,
		CustomPos = 1 << 1,
		AutoAim = 1 << 2
	};
};
using FenixProjsTypes = FenixProjs::Types;
using FenixProjsType = RE::stl::enumeration<FenixProjsTypes, uint32_t>;

bool is_CustomPosType(RE::Projectile* proj);
void set_CustomPosType(RE::Projectile* proj);
void set_NormalType(RE::Projectile* proj);

bool is_AutoAimType(RE::Projectile* proj);
void set_AutoAimType(RE::Projectile* proj);

namespace AutoAim
{
	void change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime);
}
