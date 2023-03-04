#include "FenixProjectilesAPI.h"
#include "JsonUtils.h"

void NewProjType::init(const Json::Value& item)
{
	if (item.isMember("NewProjType")) {
		auto& NewProjType = item["NewProjType"];

		auto read_field = [&NewProjType](std::string_view field_name) {
			return NewProjType.isMember(field_name.data()) ?
			           JsonUtils::get_formid(NewProjType[field_name.data()].asString()) :
			           0;
		};

		emitter = read_field("Emitter"sv);
		homing = read_field("Homing"sv);
		follower = read_field("Follower"sv);
	} else {
		*this = {};
	}
}
