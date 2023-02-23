#pragma once

#include "AutoAim.h"
#include "CustomFloats.h"

class FenixProjsRuntimeData
{
public:
	using AutoAimParam_t = CustomFloat<4, 3>;
	using EmitterInterval_t = CustomFloat<4, 4>;

	struct Types
	{
		uint32_t CustomPos: 1;        // 0:00 -- disable spawning in magicnode
		AutoAimTypes AutoAimType: 2;  // 0:01
		uint32_t AutoAimParam : AutoAimParam_t::SIZEOF;  // 0:03 -- accel or rot_speed
		uint32_t EmitterInterval : EmitterInterval_t::SIZEOF;  // 0:0B -- 0 if disabled
		uint32_t EmitterKey: 8;                                // 0:13
		uint32_t unused: 6;                                    // 0:1B

		void set_AutoAimParam(float val) { AutoAimParam = AutoAimParam_t::f2u(val); }

		float get_AutoAimParam() { return AutoAimParam_t::u2f(AutoAimParam); }

		void set_EmitterInterval(float val)
		{
			EmitterInterval = EmitterInterval_t::f2u(val);
		}

		float get_EmitterInterval() { return EmitterInterval_t::u2f(EmitterInterval); }

		uint32_t get_EmitterKey() { return EmitterKey & 0xFF; }
		void set_EmitterKey(uint32_t runtime_key) { EmitterKey = runtime_key; }
	};
	static_assert(sizeof(Types) == 0x4);

	bool isCustomPos() { return static_cast<bool>(type.CustomPos); }
	void setCustomPos() { type.CustomPos = true; }

	bool isAutoAim() { return type.AutoAimType != AutoAimTypes::Normal; }
	AutoAimTypes autoAimType() { return type.AutoAimType; }
	void set_AutoAim(AutoAimTypes aim_type) { type.AutoAimType = aim_type; }
	void set_AutoAimParam(float param) { type.set_AutoAimParam(param); }
	auto get_AutoAimParam() { return type.get_AutoAimParam(); }

	void set_EmitterInterval(float interval) { type.set_EmitterInterval(interval); }
	auto get_EmitterInterval() { return type.get_EmitterInterval(); }
	auto get_EmitterKey() { return type.get_EmitterKey(); }
	void set_EmitterKey(uint32_t runtime_key) { type.set_EmitterKey(runtime_key); }

	void set_NormalType() { (uint32_t&)type = 0; }

	Types type;
};
static_assert(sizeof(FenixProjsRuntimeData) == 0x4);

FenixProjsRuntimeData& get_runtime_data(RE::Projectile* proj);

//bool is_CustomPosType(RE::Projectile* proj);
//void set_CustomPosType(RE::Projectile* proj);
void init_NormalType(RE::Projectile* proj);
