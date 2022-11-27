#pragma once

class FenixProjsType
{
public:
	enum class AutoAimType : std::uint32_t
	{
		Normal = 0,
		ConstSpeed = 1,
		ConstAcc = 2
	};

	enum class AutoAimTarget : uint32_t
	{
		Nearest, NearestHostile, AtCursor
	};

	struct JsonData
	{
	public:
		AutoAimTarget target: 2;  // 0:00
		uint32_t param: 30;       // 0:02
	};
	static_assert(sizeof(JsonData) == 0x4);

	struct Types
	{
	public:
		uint32_t Normal: 1;          // 0:00
		uint32_t CustomPos: 1;       // 0:01
		AutoAimType AutoAimType: 2;  // 0:02
		uint32_t param: 28;          // 0:04
	};
	static_assert(sizeof(Types) == 0x4);

	bool isCustomPos() { return static_cast<bool>(type.CustomPos); }
	void setCustomPos() { type.CustomPos = true; }
	bool isAutoAim() { return type.AutoAimType != AutoAimType::Normal; }
	AutoAimType autoAimType() { return type.AutoAimType; }
	void set_AutoAim(AutoAimType aim_type) { type.AutoAimType = aim_type; }
	void set_AutoAimParam(uint32_t param) { type.param = param; }
	uint32_t get_AutoAimParam() { return type.param; }

	void set_NormalType() { (uint32_t&)type = 0; }

	Types type;
};
static_assert(sizeof(FenixProjsType) == 0x4);

using AutoAimTypes = FenixProjsType::AutoAimType;
using AutoAimTarget = FenixProjsType::AutoAimTarget;
using JsonData = FenixProjsType::JsonData;
using AutoAimData_t = std::pair<AutoAimTypes, JsonData>;

bool is_CustomPosType(RE::Projectile* proj);
void set_CustomPosType(RE::Projectile* proj);
void set_NormalType(RE::Projectile* proj);

bool is_AutoAimType(RE::Projectile* proj);
AutoAimTypes get_AutoAimType(RE::Projectile* proj);
void set_AutoAimType(RE::Projectile* proj, AutoAimData_t type);

AutoAimData_t is_homie(RE::Projectile* proj);

void read_json();

namespace AutoAim
{
	void change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime);
}
