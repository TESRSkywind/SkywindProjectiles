#include "RuntimeData.h"
#include "Multicast.h"

namespace Emitters
{
	namespace Data
	{
		class JsonStorage
		{
			// Stored in Json, queried every function call
			struct JsonDataItem
			{
				uint32_t function_key;  // key of function to call
				float interval;         // in RuntimeData
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
				data.function_key = JsonUtils::get_formid(item["key"].asString());
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
			static auto query_oncreated(uint32_t big_key) {
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
			static uint32_t query_update(uint32_t runtime_key)
			{
				if (auto found = EmittersData.find(runtime_key);
					found != EmittersData.end()) {
					return (*found).second.function_key;
				}

				return 0;
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
	}

	void call_func(RE::Projectile* proj, uint32_t function_key, RE::Actor* caster,
		RE::SpellItem* spel)
	{
		auto curpos = proj->GetPosition();

		ManyProjs::Casting::CastData cast_data;
		cast_data.startPos = &curpos;
		cast_data.caster = caster;
		cast_data.parallel_rot = { proj->data.angle.x, proj->data.angle.z };
		cast_data.spel = spel;
		ManyProjs::Casting::onManyCasted(function_key, cast_data);
	}

	void onUpdate(RE::Projectile* proj, float)
	{
		auto _caster = proj->shooter.get().get();
		if (!_caster)
			return;
		auto caster = _caster->As<RE::Actor>();
		if (!caster)
			return;

		auto spel = proj->spell->As<RE::SpellItem>();
		if (!spel)
			return;

		if (proj->livingTime < Data::get_interval(proj))
			return;

		auto runtime_key = Data::get_key(proj);
		if (runtime_key == 0)
			return;

		proj->livingTime = 0.000001f;

		auto function_key = Data::JsonStorage::query_update(runtime_key);
		call_func(proj, function_key, caster, spel);
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
				_smth_explode =
					SKSE::GetTrampoline().write_call<5>(REL::ID(42852).address() + 0x680,
						smth_explode);  // SkyrimSE.exe+745450
			}

		private:
			static RE::Explosion* smth_explode(RE::MissileProjectile* proj, float dtime)
			{
				if (Data::is_Emitter(proj)) {
					onUpdate(proj, dtime);
				}

				return _smth_explode(proj, dtime);
			}

			static inline REL::Relocation<decltype(smth_explode)> _smth_explode;
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
