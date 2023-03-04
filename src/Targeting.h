#include "JsonUtils.h"
#include "CustomFloats.h"

namespace Targeting
{
	using detection_angle_t = CustomFloat<4, 3>;

	// Stored in Json
	enum class AimTargetType : uint32_t
	{
		Nearest,
		Hostile,
		Cursor,

		Total
	};
	static constexpr AimTargetType AimTargetType__DEFAULT = AimTargetType::Nearest;

	RE::Actor* findTarget(RE::TESObjectREFR* origin, const std::tuple<AimTargetType, float>& target_data);
	
	RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime = 0.0f);

	namespace Cursor
	{
		RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle);
	}

	namespace Data
	{
		AimTargetType read_json_entry(const Json::Value& item);
		uint32_t read_detectionAngle(const Json::Value& item, AimTargetType target);
		float get_detectionAngle(uint32_t detection_angle);
	}
}
