#pragma once

#include "AutoAim.h"
#include "CustomFloats.h"

class FenixProjsRuntimeData
{
public:
	using AutoAimParam_t = CustomFloat<4, 3>;
	using EmitterInterval_t = CustomFloat<4, 4>;

	enum class MovingType: uint32_t
	{
		None, Aiming, Following
	};

	static constexpr inline uint32_t MovingDataData_sizeof = 8;

	struct MovingDataAim
	{
		AutoAimTypes AutoAimType: 1;					 // 00
		uint32_t AutoAimParam : AutoAimParam_t::SIZEOF;  // 01 -- accel or rot_speed
	};
	static_assert(1 + AutoAimParam_t::SIZEOF == MovingDataData_sizeof);

	struct MovingData
	{
		MovingType type: 2;
		uint32_t data : MovingDataData_sizeof;  // MovingDataAim or MovingDataFollow
	};
	static constexpr inline uint32_t MovingData_sizeof = 2 + MovingDataData_sizeof;

	struct Types
	{
		uint32_t moving_data : MovingData_sizeof;              // 0:00 -- Aim or Follow
		uint32_t EmitterInterval : EmitterInterval_t::SIZEOF;  // 0:0A -- 0 if disabled
		uint32_t EmitterKey: 8;                                // 0:12
		uint32_t unused: 6;                                    // 0:1A

	private:
		MovingData get_MovingData()
		{
			uint32_t val = this->moving_data;
			MovingData ans = *(MovingData*)&val;
			return ans;
		}

		void set_MovingData(MovingData val)
		{
			uint32_t ans = *(uint32_t*)&val;
			this->moving_data = ans;
		}

		MovingDataAim get_MovingDataAim() {
			MovingData data = get_MovingData();
			uint32_t val = data.data;
			MovingDataAim ans = *(MovingDataAim*)&val;
			return ans;
		}

		void set_MovingDataAim(MovingDataAim val) {
			uint32_t ans = *(uint32_t*)&val;
			MovingData data = get_MovingData();
			data.data = ans;
			set_MovingData(data);
		}

	public:
		void set_AutoAimParam(float val) {
			MovingDataAim data = get_MovingDataAim();
			data.AutoAimParam = AutoAimParam_t::f2u(val);
			set_MovingDataAim(data);
		}

		float get_AutoAimParam() {
			MovingDataAim data = get_MovingDataAim();
			return AutoAimParam_t::u2f(data.AutoAimParam);
		}

		bool isAutoAim()
		{
			MovingData data = get_MovingData();
			return data.type == MovingType::Aiming;
		}

		void disable_AutoAim()
		{
			if (!isAutoAim())
				return;

			MovingData data = get_MovingData();
			data.type = MovingType::None;
			set_MovingData(data);
		}

		void enable_AutoAim()
		{
			if (isAutoAim())
				return;

			MovingData data = get_MovingData();
			data.type = MovingType::Aiming;
			set_MovingData(data);
		}

		AutoAimTypes get_autoAimType()
		{
			MovingDataAim data = get_MovingDataAim();
			return data.AutoAimType;
		}

		void set_AutoAimType(AutoAimTypes aim_type) {
			MovingDataAim data = get_MovingDataAim();
			data.AutoAimType = aim_type;
			set_MovingDataAim(data);
		}

		void set_EmitterInterval(float val) { EmitterInterval = EmitterInterval_t::f2u(val); }

		float get_EmitterInterval() { return EmitterInterval_t::u2f(EmitterInterval); }

		uint32_t get_EmitterKey() { return EmitterKey & 0xFF; }
		void set_EmitterKey(uint32_t runtime_key) { EmitterKey = runtime_key; }
	};
	static_assert(sizeof(Types) == 0x4);

	bool isAutoAim() { return type.isAutoAim(); }
	void disable_AutoAim() { type.disable_AutoAim(); }
	void enable_AutoAim() { type.enable_AutoAim(); }
	AutoAimTypes get_autoAimType() { return type.get_autoAimType(); }
	void set_AutoAimType(AutoAimTypes aim_type) { type.set_AutoAimType(aim_type); }
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

void init_NormalType(RE::Projectile* proj);
