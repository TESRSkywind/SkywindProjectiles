#include "RuntimeData.h"
#include "Multicast.h"
#include "AutoAim.h"

namespace Emitters
{
	namespace Data
	{
		class JsonStorage
		{
		public:
			// What can happend in update
			enum class FunctionTypes : uint32_t
			{
				Multicast,
				ChangeType
			};

			using JsonDataItem_data_t = std::variant<uint32_t, NewProjType>;  // FunctionTypes

		private:
			// Stored in Json, queried every function call
			struct JsonDataItem
			{
				float interval;            // in RuntimeData
				JsonDataItem_data_t data;  // Multicast: key; ChangeType: NewProjType
			};

			using Map_t = std::unordered_map<uint32_t, JsonDataItem>;

			static inline Map_t EmittersData;  // runtime_key -> data { int, func_key }
			static inline std::unordered_map<uint32_t, uint32_t>
				EmittersMap;  // big_key -> runtime_key

			// init JsonDataItem entry and insert it to the map
			static void read_json_entry_global(const Json::Value& item,
				uint32_t runtime_key)
			{
				JsonDataItem data;
				data.interval = item["interval"].asFloat();
				FunctionTypes type =
					parse_enum<FunctionTypes::Multicast>(item["type"].asString());
				switch (type) {
				case FunctionTypes::Multicast:
					std::get<uint32_t>(data.data) =
						JsonUtils::get_formid(item["key"].asString());
					break;
				case FunctionTypes::ChangeType:
					data.data = NewProjType{};
					std::get<NewProjType>(data.data).init(item);
					break;
				default:
					break;
				}

				EmittersData.insert({ runtime_key, data });
			}

			// insert pair formid, key to the map
			static void read_json_entry(const Json::Value& item, uint32_t formid)
			{
				uint32_t runtime_key = JsonUtils::get_formid(item["key"].asString());
				EmittersMap.insert({ formid, runtime_key });
			}

		public:
			static void init(const Json::Value& emitters, int hex)
			{
				for (auto& formid : emitters.getMemberNames()) {
					read_json_entry(emitters[formid],
						hex | JsonUtils::get_formid(formid));
				}
			}

			static void init_global(const Json::Value& emitters)
			{
				for (auto& key : emitters.getMemberNames()) {
					read_json_entry_global(emitters[key], JsonUtils::get_formid(key));
				}
			}

			static void clear()
			{
				EmittersMap.clear();
				EmittersData.clear();
			}

			// interval, runtime_key
			static auto query_oncreated(uint32_t big_key)
			{
				std::tuple<float, uint32_t> ans = {};
				if (auto found = EmittersMap.find(big_key); found != EmittersMap.end()) {
					auto runtime_key = (*found).second;
					if (auto found1 = EmittersData.find(runtime_key);
						found1 != EmittersData.end()) {
						auto interval = (*found1).second.interval;

						std::get<float>(ans) = interval;
						std::get<uint32_t>(ans) = runtime_key;
					}
				}
				return ans;
			}

			// function_key
			static JsonDataItem_data_t query_update(uint32_t runtime_key)
			{
				if (auto found = EmittersData.find(runtime_key);
					found != EmittersData.end()) {
					return (*found).second.data;
				}

				return JsonDataItem_data_t{};
			}
		};

		auto get_interval(RE::Projectile* proj)
		{
			return get_runtime_data(proj).get_EmitterInterval();
		}

		auto get_key(RE::Projectile* proj)
		{
			return get_runtime_data(proj).get_EmitterKey();
		}

		bool is_Emitter(RE::Projectile* proj) { return get_interval(proj) > 0; }

		void set_normalType(RE::Projectile* proj)
		{
			get_runtime_data(proj).set_EmitterKey(0);
			get_runtime_data(proj).set_EmitterInterval(0);
		}
	}

	void onCreated(RE::Projectile* proj, uint32_t big_key);

	auto rot_at(RE::NiPoint3 dir)
	{
		ProjectileRot rot;
		auto len = dir.Unitize();
		if (len == 0) {
			rot = { 0, 0 };
		} else {
			float polar_angle = _generic_foo_<68820, float(RE::NiPoint3 * p)>::eval(&dir);  // SkyrimSE.exe+c51f70
			rot = { -asin(dir.z), polar_angle };
		}

		return rot;
	}

	auto rot_at(const RE::NiPoint3& from, const RE::NiPoint3& to) { return rot_at(to - from); }

	void call_func(RE::Projectile* proj, Data::JsonStorage::JsonDataItem_data_t data,
		RE::Actor* caster)
	{
		using FunctionTypes = Data::JsonStorage::FunctionTypes;

		FunctionTypes type = (FunctionTypes)data.index();
		switch (type) {
		case FunctionTypes::Multicast:
			{
				auto function_key = std::get<uint32_t>(data);
				auto curpos = proj->GetPosition();

				ManyProjs::Casting::CastData cast_data;
				cast_data.startPos = &curpos;
				cast_data.caster = caster;
				cast_data.parallel_rot = rot_at(proj->linearVelocity);

				if (auto ammo = proj->ammoSource) {
					cast_data.data = ManyProjs::Casting::CastData::ArrowData{ proj->weaponSource, ammo };
				} else if (auto spel = proj->spell) {
					cast_data.data = ManyProjs::Casting::CastData::SpellData{ spel->As<RE::SpellItem>() };
				} else {
					cast_data.data = ManyProjs::Casting::CastData::SpellData{ nullptr };
				}

				ManyProjs::Casting::onManyCasted(function_key, cast_data);
			}
			break;
		case FunctionTypes::ChangeType:
			{
				auto new_types = std::get<NewProjType>(data);

				auto emitter_key = new_types.emitter;
				if (emitter_key != 0) {
					if (emitter_key == -1) {
						Data::set_normalType(proj);
					} else {
						onCreated(proj, emitter_key);
					}
				}

				auto homing_key = new_types.homing;
				if (homing_key != 0) {
					if (homing_key == -1) {
						AutoAim::Data::set_NormalType(proj);
					} else {
						AutoAim::onCreated(proj, homing_key);
					}
				}
			}
			break;
		default:
			break;
		}
	}

	void onUpdate(RE::Projectile* proj, float)
	{
		auto _caster = proj->shooter.get().get();
		if (!_caster || !_caster->As<RE::Actor>() || proj->distanceMoved > 10000) {
			Data::set_normalType(proj);
			return;
		}

		auto caster = _caster->As<RE::Actor>();

		if (proj->livingTime < Data::get_interval(proj))
			return;

		auto runtime_key = Data::get_key(proj);
		if (runtime_key == 0)
			return;

		proj->livingTime = 0.000001f;

		auto data = Data::JsonStorage::query_update(runtime_key);
		call_func(proj, data, caster);
	}

	void onCreated(RE::Projectile* proj, uint32_t big_key) {
		auto data = Data::JsonStorage::query_oncreated(big_key);

		get_runtime_data(proj).set_EmitterInterval(std::get<float>(data));
		get_runtime_data(proj).set_EmitterKey(std::get<uint32_t>(data));
	}

	void onCreated(RE::Projectile* proj)
	{
		return onCreated(proj, proj->GetBaseObject()->formID);
	}

	namespace Hooks
	{
		class EmitterHook
		{
		public:
			static void Hook()
			{
				_smth_explode = SKSE::GetTrampoline().write_call<5>(REL::ID(42852).address() + 0x680,
					smth_explode);  // SkyrimSE.exe+745450
				_AddImpact = SKSE::GetTrampoline().write_call<5>(REL::ID(42547).address() + 0x56,
					AddImpact);  // SkyrimSE.exe+745450 -- disable on hit
				_sub_140BEDB60 = SKSE::GetTrampoline().write_call<5>(REL::ID(42930).address() + 0x21,
					sub_140BEDB60);  // SkyrimSE.exe+74BC21 -- disable on destroy
			}

		private:
			static RE::Explosion* smth_explode(RE::MissileProjectile* proj, float dtime)
			{
				if (Data::is_Emitter(proj)) {
					onUpdate(proj, dtime);
				}

				return _smth_explode(proj, dtime);
			}

			static void* AddImpact(RE::Projectile* proj, RE::TESObjectREFR* a2, RE::NiPoint3* a3, RE::NiPoint3* a_velocity,
				RE::hkpCollidable* a_collidable, uint32_t a6, char a7)
			{
				auto ans = _AddImpact(proj, a2, a3, a_velocity, a_collidable, a6, a7);
				if (Data::is_Emitter(proj)) {
					Data::set_normalType(proj);
				}
				return ans;
			}

			static void sub_140BEDB60(char* sound) {
				_sub_140BEDB60(sound);
				auto proj = reinterpret_cast<RE::Projectile*>(sound - 0x128);
				if (Data::is_Emitter(proj)) {
					Data::set_normalType(proj);
				}
			}

			static inline REL::Relocation<decltype(smth_explode)> _smth_explode;
			static inline REL::Relocation<decltype(AddImpact)> _AddImpact;
			static inline REL::Relocation<decltype(sub_140BEDB60)> _sub_140BEDB60;
		};
	}

	void forget() { Data::JsonStorage::clear(); }

	void add_mod(const Json::Value& homing, int hex)
	{
		Data::JsonStorage::init(homing, hex);
	}

	void install()
	{
		Hooks::EmitterHook::Hook();
	}

	void init_global(const Json::Value& json_root)
	{
		if (json_root.isMember("Emitters"))
			Data::JsonStorage::init_global(json_root["Emitters"]);
	}
}
