#include "Following.h"
#include "RuntimeData.h"
#include "Settings.h"

namespace Following
{
	// Stored in Json
	enum class FollowingCaster : uint32_t
	{
		Both,
		Player,
		NPC
	};
	static constexpr FollowingCaster FollowingCaster__DEFAULT = FollowingCaster::Both;

	namespace Data
	{
		class JsonStorage
		{
		public:
			using detection_angle_t = CustomFloat<4, 3>;
			using acc_or_rottime_t = FenixProjsRuntimeData::AutoAimParam_t;

			// Stored in Json (in 2 maps), queried only at creation
			struct JsonDataItem
			{
				uint32_t size;           // 00
				uint32_t index;          // 04
				FollowTypes type;        // 08
				FollowingCaster caster;  // 0C
			};
			static_assert(sizeof(JsonDataItem) == 0x10);

			static constexpr JsonDataItem JsonDataItem__DEFAULT = {};

		private:
			using Map_t = std::unordered_map<uint32_t, JsonDataItem>;

			static inline Map_t FollowingMap;

			// init JsonDataItem entry and insert it to the map
			static void read_json_entry(const Json::Value& item, uint32_t formid)
			{
				JsonDataItem data;
				data.size = item["size"].asUInt();
				data.index = item["index"].asUInt();
				data.type = parse_enum<FollowTypes::Nimbus>(item["type"].asString());
				data.caster = parse_enum_ifIsMember<FollowingCaster__DEFAULT>(item, "caster"sv);

				FollowingMap.insert({ formid, data });
			}

			// Lookup specific map
			static auto query_map(const Map_t& storage, uint32_t key) { return storage.find(key); }

		public:
			static void init(const Json::Value& following, int hex)
			{
				for (auto& formid : following.getMemberNames()) {
					read_json_entry(following[formid], hex | JsonUtils::get_formid(formid));
				}
			}

			static void clear() { FollowingMap.clear(); }

			// Lookup both maps or return default entry
			static std::tuple<bool, JsonDataItem> query(uint32_t key)
			{
				if (auto found = query_map(FollowingMap, key); found != FollowingMap.end()) {
					return { true, (*found).second };
				}

				return { false, JsonDataItem__DEFAULT };
			}
		};

		// Set runtime data type and param
		void set_FollowType(RE::Projectile* proj, FollowTypes type, uint32_t size, uint32_t index)
		{
			get_runtime_data(proj).enable_Follow();
			get_runtime_data(proj).set_FollowType(type);
			get_runtime_data(proj).set_FollowSize(size);
			get_runtime_data(proj).set_FollowIndex(index);
		}

		auto get_FollowSize(RE::Projectile* proj) { return get_runtime_data(proj).get_FollowSize(); }
		auto get_FollowIndex(RE::Projectile* proj) { return get_runtime_data(proj).get_FollowIndex(); }
		bool is_FollowType(RE::Projectile* proj) { return get_runtime_data(proj).isFollow(); }

		// 3rd is size, 4th is index
		auto get_onCreate_data(uint32_t key)
		{
			std::tuple<bool, FollowingCaster, uint32_t, uint32_t> ans;
		
			auto data = JsonStorage::query(key);
			const auto& json_data = std::get<JsonStorage::JsonDataItem>(data);
			std::get<bool>(ans) = std::get<bool>(data);
			std::get<2>(ans) = json_data.size;
			std::get<3>(ans) = json_data.index;
		
			return ans;
		}
	}
	using Data::get_onCreate_data;

	namespace Moving
	{
		// SkyrimSE.exe+74DC20
		float get_proj_speed(RE::Projectile* proj) { return _generic_foo_<42958, decltype(get_proj_speed)>::eval(proj); }

		RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime)
		{
			RE::NiPoint3 ans, eye_pos;
			target->GetLinearVelocity(ans);
			ans *= dtime;
			FenixUtils::Actor__get_eye_pos(target, eye_pos, 3);
			ans += eye_pos;
			return ans;
		}

		void SetRotationMatrix(RE::NiMatrix3& a_matrix, float sacb, float cacb, float sb)
		{
			float cb = std::sqrtf(1 - sb * sb);
			float ca = cacb / cb;
			float sa = -sacb / cb;
			a_matrix.entry[0][0] = ca;
			a_matrix.entry[0][1] = sacb;
			a_matrix.entry[0][2] = sa * sb;
			a_matrix.entry[1][0] = sa;
			a_matrix.entry[1][1] = cacb;
			a_matrix.entry[1][2] = -ca * sb;
			a_matrix.entry[2][0] = 0.0;
			a_matrix.entry[2][1] = sb;
			a_matrix.entry[2][2] = cb;
		}

		void update_node_rotation(RE::Projectile* proj)
		{
			RE::NiPoint3 proj_dir = proj->linearVelocity;
			proj_dir.Unitize();

			proj->data.angle.x = asin(proj_dir.z);
			proj->data.angle.z = atan2(proj_dir.x, proj_dir.y);

			if (proj_dir.x < 0.0) {
				proj->data.angle.x += 3.1415926f;
			}

			if (proj->data.angle.z < 0.0) {
				proj->data.angle.z += 3.1415926f;
			}

			SetRotationMatrix(proj->Get3D2()->local.rotate, proj_dir.x, proj_dir.y, proj_dir.z);
		}

		void change_direction_linVel(RE::Projectile* proj, float)
		{
			RE::NiPoint3 final_vel;
			auto _caster = proj->shooter.get().get();
			if (!_caster)
				return;
			auto caster = _caster->As<RE::Actor>();

			auto spawn_center = caster->GetPosition() + RE::NiPoint3{0,0,200};
			auto cast_dir = FenixUtils::rotate(1, caster->data.angle);

			RE::NiPoint3 right_dir = RE::NiPoint3(0, 0, -1).UnitCross(cast_dir);
			if (right_dir.SqrLength() == 0)
				right_dir = { 1, 0, 0 };
			RE::NiPoint3 up_dir = right_dir.Cross(cast_dir);

			uint32_t count = Data::get_FollowSize(proj);
			auto Foo = [&spawn_center, &right_dir, &up_dir, &count]() -> std::vector<RE::NiPoint3> {
				std::vector<RE::NiPoint3> ans;
				float size = 100.0f;
				for (uint32_t i = 0; i < count; i++) {
					float alpha = 2 * 3.1415926f / count * i;

					auto cur_p = spawn_center + right_dir * cos(alpha) * size + up_dir * sin(alpha) * size;

					ans.push_back(cur_p);
				}
				return ans;
			};

			auto points = Foo();
			uint32_t ind = Data::get_FollowIndex(proj);
			const auto& target_pos = points[ind];

#ifdef DEBUG
			{
				for (const auto& i : points) {
					draw_point0(i);
				}
			}
#endif  // DEBUG

			auto dir = target_pos - proj->GetPosition();
			auto dist = dir.Unitize();
			auto speed_origin = get_proj_speed(proj);
			auto speed_needed = dist * 1.0f;
			float speed = std::min(speed_origin, speed_needed);
			proj->linearVelocity = dir * speed;
		}

		void change_direction(RE::Projectile* proj, RE::NiPoint3*, float dtime)
		{
			change_direction_linVel(proj, dtime);

			update_node_rotation(proj);

#ifdef DEBUG
			{
				auto proj_dir = proj->linearVelocity;
				proj_dir.Unitize();
				draw_line<Colors::RED>(proj->GetPosition(), proj->GetPosition() + proj_dir);
			}
#endif  // DEBUG
		}
	}

	namespace Hooks
	{
		// Make projectile follow the caster
		class FollowingHook
		{
		public:
			static void Hook()
			{
				_Projectile__apply_gravity = SKSE::GetTrampoline().write_call<5>(REL::ID(43006).address() + 0x69,
					change_direction);  // SkyrimSE.exe+751309
			}

		private:
			static bool change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime)
			{
				bool ans = _Projectile__apply_gravity(proj, dV, dtime);
				if (Data::is_FollowType(proj)) {
					Moving::change_direction(proj, dV, dtime);
				}
				return ans;
			}

			static inline REL::Relocation<decltype(change_direction)> _Projectile__apply_gravity;
		};
	}

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

	uint32_t get_min_unused_index_ofType(RE::Projectile* proj, const RE::BSTArray<RE::ProjectileHandle>& a)
	{
		uint32_t baseID = proj->GetBaseObject()->formID;
		uint32_t ans = 0;
		for (auto& i : a) {
			if (auto _proj = i.get().get(); _proj && _proj->GetBaseObject()->formID == baseID && _proj->formID != proj->formID) {
				uint32_t ind = Data::get_FollowIndex(_proj);
				ans = ind + 1;
			}
		}
		return ans;
	}

	uint32_t get_min_unused_index_ofType(RE::Projectile* proj)
	{
		auto manager = RE::Projectile::Manager::GetSingleton();
		uint32_t limitedID = get_min_unused_index_ofType(proj, manager->limited);
		uint32_t pendingID = get_min_unused_index_ofType(proj, manager->pending);
		uint32_t unlimitedID = get_min_unused_index_ofType(proj, manager->unlimited);
		uint32_t freeID = std::max(limitedID, pendingID);
		freeID = std::max(freeID, unlimitedID);
		return freeID;
	}

	void onCreated(RE::Projectile* proj, uint32_t key, int ind)
	{
		if (auto data = get_onCreate_data(key); std::get<bool>(data)) {
			if (auto caster = proj->shooter.get().get()) {
				bool isPlayer = caster->IsPlayerRef();
				auto caster_type = std::get<FollowingCaster>(data);
				if (caster_type == FollowingCaster::NPC && !isPlayer || caster_type == FollowingCaster::Player && isPlayer ||
					caster_type == FollowingCaster::Both) {
					if (proj->IsMissileProjectile()) {
						uint32_t size = std::get<2>(data);
						//uint32_t index = std::get<3>(data);
						//uint32_t index = get_min_unused_index_ofType(proj);
						uint32_t index = ind == -1 ? std::get<3>(data) : ind;
						if (index < size)
							Data::set_FollowType(proj, FollowTypes::Nimbus, size, index);
					}
				}
			}
		}
	}

	void onCreated(RE::Projectile* proj) { return onCreated(proj, proj->GetBaseObject()->formID); }

	void forget() { Data::JsonStorage::clear(); }

	void add_mod(const Json::Value& homing, int hex) { Data::JsonStorage::init(homing, hex); }

	void install()
	{
		Hooks::FollowingHook::Hook();
	}
}
