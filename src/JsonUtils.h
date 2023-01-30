#pragma once

#include "FenixProjectilesAPI.h"

template <typename T, T def>
T parse_impl(const std::string& s)
{
	return magic_enum::enum_cast<T>(s).value_or(def);
}

template <auto def>
auto parse_enum(const std::string& s)
{
	return parse_impl<decltype(def), def>(s);
}

template <auto def>
static auto parse_enum_ifIsMember(const Json::Value& item, std::string_view field_name)
{
	if constexpr (std::is_same<decltype(def), uint32_t>::value) {
		if (item.isMember(field_name.data()))
			return item[field_name.data()].asUInt();
		else
			return def;
	} else {
		if (item.isMember(field_name.data()))
			return parse_enum<def>(item[field_name.data()].asString());
		else
			return def;
	}
}

namespace JsonUtils
{
	RE::NiPoint3 get_point(const Json::Value& point);
	ProjectileRot get_point2(const Json::Value& point);

	// returns prefix of the forms defined by this mod
	int get_mod_index(const std::string_view& name);

	// Get runtime formid by 0x314 or F.esp|0x314 or key_f314
	uint32_t get_formid(const std::string& name);

	class FormIDsMap
	{
		static inline std::unordered_map<std::string, uint32_t> formIDs;

	public:
		static void init(const Json::Value& json_root)
		{
			formIDs.clear();

			if (!json_root.isMember("FormIDs"))
				return;

			const auto& formids = json_root["FormIDs"];
			for (auto& key : formids.getMemberNames()) {
				formIDs.insert({ key, get_formid(formids[key].asString()) });
			}
		}

		// `key` must present and starts with "key_"
		static auto get(std::string key)
		{
			auto found = formIDs.find(key);
			return found == formIDs.end() ? 0 : (*found).second;
		}
	};
}
