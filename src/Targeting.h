#include "JsonUtils.h"

namespace Targeting
{
	// Stored in Json
	enum class AimTargetType : uint32_t
	{
		Nearest,
		Hostile,
		Cursor
	};
	static constexpr AimTargetType AimTargetType__DEFAULT = AimTargetType::Nearest;

	RE::Actor* findTarget(RE::Projectile* proj, const std::tuple<AimTargetType, float>& target_data);

	namespace Cursor
	{
		RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle);
	}

	namespace Data
	{
		AimTargetType read_json_entry(const Json::Value& item);
	}
}
