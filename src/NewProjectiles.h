#pragma once
#include "FenixProjectilesAPI.h"

class FenixProjsRuntimeData
{
public:
	enum class AutoAimType : std::uint32_t
	{
		Normal = 0,
		ConstSpeed = 1,
		ConstAcc = 2
	};

	struct Types
	{
	public:
		uint32_t CustomPos: 1;       // 0:00
		AutoAimType AutoAimType: 2;  // 0:01
		uint32_t param: 29;          // 0:03
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
static_assert(sizeof(FenixProjsRuntimeData) == 0x4);

bool is_CustomPosType(RE::Projectile* proj);
void set_CustomPosType(RE::Projectile* proj);
void set_NormalType(RE::Projectile* proj);

void read_json();

namespace AutoAim
{
	class FenixProjsAutoAimData
	{
	public:
		enum class AutoAimTarget : uint32_t
		{
			Nearest,
			NearestHostile,
			AtCursor
		};

		enum class AutoAimCaster : uint32_t
		{
			Both,
			Player,
			NPC
		};

		struct JsonData
		{
		public:
			AutoAimCaster caster: 2;  // 0:00
			AutoAimTarget target: 2;  // 0:02
			uint32_t param2: 8;       // 0:04 - angle for cursor target
			uint32_t param1: 20;      // 0:12 - acceleration for consAccel & rotation time for constSpeed
		};
		static_assert(sizeof(JsonData) == 0x4);
	};
	using AutoAimTarget = FenixProjsAutoAimData::AutoAimTarget;
	using AutoAimCaster = FenixProjsAutoAimData::AutoAimCaster;
	using JsonData = FenixProjsAutoAimData::JsonData;
	using AutoAimTypes = FenixProjsRuntimeData::AutoAimType;
	using AutoAimData_t = std::pair<AutoAimTypes, JsonData>;

	bool is_AutoAimType(RE::Projectile* proj);
	AutoAimTypes get_AutoAimType(RE::Projectile* proj);
	void set_AutoAimType(RE::Projectile* proj, AutoAimData_t type);
	void change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime);
	RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle);
	AutoAimData_t is_homie(RE::TESBoundObject* proj);
}
using AutoAim::is_AutoAimType;

namespace ManyProjs
{
	class FenixProjsMultiCastData
	{
	public:
		enum class Shape : uint32_t
		{
			Single,
			HorisontalLine,
			VerticalLine,
			Circle,
			FillSquare,
			FillSphere,

			Total
		};

		enum class LaunchType : uint32_t
		{
			Parallel,
			ToSight
		};

		struct MultiCastJsonData
		{
		public:
			Shape shape: 3;        // 0:00
			LaunchType launch: 2;  // 0:03
			uint32_t size: 12;     // 0:05
			uint32_t count: 15;    // 0:05
		};
		static_assert(sizeof(MultiCastJsonData) == 0x4);
	};
	using JsonData = FenixProjsMultiCastData::MultiCastJsonData;
	using Shape = FenixProjsMultiCastData::Shape;
	using LaunchType = FenixProjsMultiCastData::LaunchType;

	bool is_ManyCast(RE::TESBoundObject* _bproj);

	void onManyCasted(RE::MagicCaster* magic_caster, RE::Actor* caster, RE::NiPoint3* startPos, const ProjectileRot rot);
}
