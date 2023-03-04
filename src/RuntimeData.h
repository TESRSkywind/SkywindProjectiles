#pragma once

#include "AutoAim.h"
#include "Following.h"
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

	static constexpr inline uint32_t MovingDataFollowSize_sizeof = (MovingDataData_sizeof - 2) / 2;
	struct MovingDataFollow
	{
		FollowTypes type: 2;                           // 00
		uint32_t size : MovingDataFollowSize_sizeof;   // 02
		uint32_t index : MovingDataFollowSize_sizeof;  // 05
	};
	static_assert(2 + MovingDataFollowSize_sizeof + MovingDataFollowSize_sizeof == MovingDataData_sizeof);

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

		MovingDataFollow get_MovingDataFollow()
		{
			MovingData data = get_MovingData();
			uint32_t val = data.data;
			MovingDataFollow ans = *(MovingDataFollow*)&val;
			return ans;
		}

		void set_MovingDataFollow(MovingDataFollow val)
		{
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

		bool isFollow()
		{
			MovingData data = get_MovingData();
			return data.type == MovingType::Following;
		}

		void enable_Follow()
		{
			if (isFollow())
				return;

			MovingData data = get_MovingData();
			data.type = MovingType::Following;
			set_MovingData(data);
		}

		void set_FollowType(FollowTypes follow_type)
		{
			MovingDataFollow data = get_MovingDataFollow();
			data.type = follow_type;
			set_MovingDataFollow(data);
		}

		void set_FollowSize(uint32_t size)
		{
			MovingDataFollow data = get_MovingDataFollow();
			data.size = size;
			set_MovingDataFollow(data);
		}

		void set_FollowIndex(uint32_t index)
		{
			MovingDataFollow data = get_MovingDataFollow();
			data.index = index;
			set_MovingDataFollow(data);
		}

		FollowTypes get_FollowType()
		{
			MovingDataFollow data = get_MovingDataFollow();
			return data.type;
		}

		uint32_t get_FollowSize()
		{
			MovingDataFollow data = get_MovingDataFollow();
			uint32_t ans = data.size;
			if (ans == 0)
				return 1 << MovingDataFollowSize_sizeof;
			else
				return ans;
		}

		uint32_t get_FollowIndex()
		{
			MovingDataFollow data = get_MovingDataFollow();
			return data.index;
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

	bool isFollow() { return type.isFollow(); }
	void enable_Follow() { type.enable_Follow(); }
	void set_FollowType(FollowTypes aim_type) { type.set_FollowType(aim_type); }
	void set_FollowSize(uint32_t size) { type.set_FollowSize(size); }
	void set_FollowIndex(uint32_t index) { type.set_FollowIndex(index); }
	FollowTypes get_FollowType() { return type.get_FollowType(); }
	uint32_t get_FollowSize() { return type.get_FollowSize(); }
	uint32_t get_FollowIndex() { return type.get_FollowIndex(); }

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
