#include "AutoAim.h"
#include "Settings.h"
#include "RuntimeData.h"

namespace AutoAim
{
	// Stored in Json
	enum class AutoAimTarget : uint32_t
	{
		Nearest,
		Hostile,
		Cursor
	};
	static constexpr AutoAimTarget AutoAimTarget__DEFAULT = AutoAimTarget::Nearest;

	// Stored in Json
	enum class AutoAimCaster : uint32_t
	{
		Both,
		Player,
		NPC
	};
	static constexpr AutoAimCaster AutoAimCaster__DEFAULT = AutoAimCaster::Both;

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
				AutoAimCaster caster: 2;  // 0:00
				AutoAimTarget target: 2;  // 0:02
				uint32_t detection_angle :
					detection_angle_t::SIZEOF;  // 0:04 - for cursor target (0..90)
				uint32_t acc_or_rottime :
					acc_or_rottime_t::
						SIZEOF;  // 0:0B - acc (* 0.1) or rottime (*0.01). in RuntimeData
				uint32_t unused: 14;  // 0:12

				void set_detectionAngle(float val)
				{
					detection_angle = detection_angle_t::f2u(val);
				}

				float get_detectionAngle() const
				{
					return detection_angle_t::u2f(detection_angle);
				}

				float get_accOrRottime(AutoAimTypes type) const
				{
					float ans = acc_or_rottime_t::u2f(acc_or_rottime);
					if (type == AutoAimTypes::ConstAccel)
						ans *= 100.0f;
					if (type == AutoAimTypes::ConstSpeed)
						ans *= 10.0f;
					return ans;
				}

				void set_accOrRottime(float val, AutoAimTypes type)
				{
					if (type == AutoAimTypes::ConstAccel)
						val *= 0.01f;
					if (type == AutoAimTypes::ConstSpeed)
						val *= 0.1f;
					acc_or_rottime = acc_or_rottime_t::f2u(val);
				}
			};
			static_assert(sizeof(JsonDataItem) == 0x4);

			static constexpr JsonDataItem JsonDataItem__DEFAULT = {};

		private:
			using Map_t = std::unordered_map<uint32_t, JsonDataItem>;

			static inline Map_t ConstSpeed;  // Const Speed
			static inline Map_t ConstAccel;  // Const Acceleration

			// init JsonDataItem entry and insert it to the according map
			static void read_json_entry(const Json::Value& item, uint32_t formid)
			{
				JsonDataItem data;
				data.caster =
					parse_enum_ifIsMember<AutoAimCaster__DEFAULT>(item, "caster"sv);
				data.target =
					parse_enum_ifIsMember<AutoAimTarget__DEFAULT>(item, "target"sv);

				data.set_detectionAngle(data.target == AutoAimTarget::Cursor ?
											item["cursorAngle"].asFloat() :
											0.0f);

				auto type = parse_enum<AutoAimTypes__DEFAULT>(item["type"].asString());

				float acc_or_rottime = 0.0f;
				switch (type) {
				case AutoAimTypes::ConstAccel:
					acc_or_rottime = item["acceleration"].asFloat();
					break;
				case AutoAimTypes::ConstSpeed:
					acc_or_rottime = item["rotationTime"].asFloat();
					break;
				default:
					break;
				}
				data.set_accOrRottime(acc_or_rottime, type);

				switch (type) {
				case AutoAimTypes::ConstAccel:
					ConstAccel.insert({ formid, data });
					break;
				case AutoAimTypes::ConstSpeed:
					ConstSpeed.insert({ formid, data });
					break;
				default:
					break;
				}
			}

			// Lookup specific map
			static auto query_map(const Map_t& storage, uint32_t key)
			{
				return storage.find(key);
			}

		public:
			static void init(const Json::Value& homing, int hex)
			{
				for (auto& formid : homing.getMemberNames()) {
					read_json_entry(homing[formid], hex | JsonUtils::get_formid(formid));
				}
			}

			static void clear()
			{
				ConstSpeed.clear();
				ConstAccel.clear();
			}

			// Lookup both maps or return default entry
			static std::tuple<bool, AutoAimTypes, JsonDataItem> query(uint32_t key)
			{
				if (auto found = query_map(ConstAccel, key); found != ConstAccel.end()) {
					return { true, AutoAimTypes::ConstAccel, (*found).second };
				}

				if (auto found = query_map(ConstSpeed, key); found != ConstSpeed.end()) {
					return { true, AutoAimTypes::ConstSpeed, (*found).second };
				}

				return { false, AutoAimTypes::ConstAccel, JsonDataItem__DEFAULT };
			}
		};

		// Set runtime data type and param
		void set_AutoAimType(RE::Projectile* proj, AutoAimTypes type,
			float acc_or_speed)
		{
			get_runtime_data(proj).enable_AutoAim();
			get_runtime_data(proj).set_AutoAimType(type);
			get_runtime_data(proj).set_AutoAimParam(acc_or_speed);
		}

		void disable_AutoAim(RE::Projectile* proj)
		{
			get_runtime_data(proj).disable_AutoAim();
		}

		// 4rd is acc_or_rottime
		auto get_onCreate_data(uint32_t key)
		{
			std::tuple<bool, AutoAimTypes, AutoAimCaster, float> ans;

			auto data = JsonStorage::query(key);
			std::get<bool>(ans) = std::get<bool>(data);
			std::get<AutoAimTypes>(ans) = std::get<AutoAimTypes>(data);
			const auto& json_data = std::get<JsonStorage::JsonDataItem>(data);
			std::get<AutoAimCaster>(ans) = json_data.caster;
			std::get<float>(ans) = json_data.get_accOrRottime(std::get<AutoAimTypes>(data));

			return ans;
		}

		// 2nd is param2 (angle)
		auto get_findTarget_data(RE::TESBoundObject* bproj)
		{
			std::tuple<AutoAimTarget, float> ans;

			auto data = JsonStorage::query(bproj->formID);
			const auto& json_data = std::get<JsonStorage::JsonDataItem>(data);
			std::get<AutoAimTarget>(ans) = json_data.target;
			std::get<float>(ans) = json_data.get_detectionAngle();

			return ans;
		}

		AutoAimTypes get_AutoAimType(RE::Projectile* proj) { return get_runtime_data(proj).get_autoAimType(); }
		auto get_AutoAimParam(RE::Projectile* proj) { return get_runtime_data(proj).get_AutoAimParam(); }
		static bool is_AutoAimType(RE::Projectile* proj) { return get_runtime_data(proj).isAutoAim(); }
	}
	using Data::get_findTarget_data;
	using Data::get_onCreate_data;

	namespace Targeting
	{
		namespace Cursor
		{
			static bool is_anglebetween_less(const RE::NiPoint3& A,
				const RE::NiPoint3& B1, const RE::NiPoint3& B2, float angle_deg)
			{
				auto AB1 = B1 - A;
				auto AB2 = B2 - A;
				AB1.Unitize();
				AB2.Unitize();
				return acos(AB1.Dot(AB2)) < angle_deg / 180.0f * 3.1415926f;
			}

			static bool is_near_to_cursor(RE::Actor* caster, RE::Actor* target,
				float angle)
			{
				RE::NiPoint3 caster_pos, caster_sight, target_pos;

				FenixUtils::Actor__get_eye_pos(caster, caster_pos, 2);
				FenixUtils::Actor__get_eye_pos(target, target_pos, 3);

				caster_sight = caster_pos;
				caster_sight += FenixUtils::rotate(1, caster->data.angle);

				return is_anglebetween_less(caster_pos, caster_sight, target_pos, angle);
			}

			enum class LineOfSightLocation : std::uint32_t
			{
				kNone = 0,
				kEyes = 1,   // Eye level
				kHead = 2,   // 85%
				kTorso = 3,  // 50%
				kFeet = 4    // 15%
			};
			static_assert(sizeof(LineOfSightLocation) == 0x4);

			static LineOfSightLocation IsActorInLineOfSight(RE::Actor* caster,
				RE::Actor* target, float viewCone = 100)
			{
				return _generic_foo_<36752, decltype(IsActorInLineOfSight)>::eval(caster,
					target, viewCone);
			}

			RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle)
			{
				if (!_caster->IsPlayerRef())
					return nullptr;

				auto caster = _caster->As<RE::Actor>();
				std::vector<std::pair<RE::Actor*, float>> targets;

				RE::TES::GetSingleton()->ForEachReference(
					[caster, &targets, angle](RE::TESObjectREFR& _refr) {
						if (auto refr = _refr.As<RE::Actor>();
							refr && !_refr.IsDisabled() && !_refr.IsDead() &&
							_refr.GetFormType() == RE::FormType::ActorCharacter &&
							_refr.formID != caster->formID &&
							is_near_to_cursor(caster, refr, angle)) {
							if (IsActorInLineOfSight(caster, refr) !=
								LineOfSightLocation::kNone) {
								targets.push_back(
									{ refr, caster->GetPosition().GetSquaredDistance(
												refr->GetPosition()) });
							}
						}
						return RE::BSContainer::ForEachResult::kContinue;
					});

				if (!targets.size())
					return nullptr;

				return (*std::min_element(targets.begin(), targets.end(),
							[](const std::pair<RE::Actor*, float>& a,
								const std::pair<RE::Actor*, float>& b) {
								return a.second < b.second;
							}))
				    .first;
			}
		}

		bool is_hostile(RE::TESObjectREFR* refr, RE::TESObjectREFR* _caster)
		{
			auto target = refr->As<RE::Actor>();
			auto caster = _caster->As<RE::Actor>();
			if (!target || !caster)
				return false;

			return target->currentCombatTarget.get().get() == caster;
		}

		RE::TESObjectREFR* find_nearest_target(RE::TESObjectREFR* caster,
			RE::Projectile* proj, bool onlyHostile)
		{
			float mindist2 = 1.0E15f;
			RE::TESObjectREFR* refr = nullptr;
			RE::TES::GetSingleton()->ForEachReference(
				[=, &mindist2, &refr](RE::TESObjectREFR& _refr) {
					if (!_refr.IsDisabled() && !_refr.IsDead() &&
						_refr.GetFormType() == RE::FormType::ActorCharacter &&
						_refr.formID != caster->formID &&
						(!onlyHostile || is_hostile(&_refr, caster))) {
						float curdist =
							proj->GetPosition().GetSquaredDistance(_refr.GetPosition());
						if (curdist < mindist2) {
							mindist2 = curdist;
							refr = &_refr;
						}
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});

			if (!refr || mindist2 > proj->range * proj->range)
				return nullptr;

			return refr;
		}

		RE::Actor* findTarget(RE::Projectile* proj)
		{
			auto target = proj->desiredTarget.get().get();
			if (target)
				return target->As<RE::Actor>();

			auto caster = proj->shooter.get().get();
			if (!caster)
				return nullptr;

			if (auto caster_npc = caster->As<RE::Actor>();
				caster_npc && !caster_npc->IsPlayerRef()) {
				return caster_npc->currentCombatTarget.get().get();
			}

			RE::TESObjectREFR* refr;
			auto data = get_findTarget_data(proj->GetBaseObject());
			auto target_type = std::get<AutoAimTarget>(data);
			switch (target_type) {
			case AutoAimTarget::Nearest:
			case AutoAimTarget::Hostile:
				refr = find_nearest_target(caster, proj,
					target_type == AutoAimTarget::Hostile);
				break;
			case AutoAimTarget::Cursor:
				refr = Cursor::find_cursor_target(caster, std::get<float>(data));
				break;
			default:
				refr = nullptr;
				break;
			}

			if (!refr)
				return nullptr;

#ifdef DEBUG
			FenixUtils::notification("Target found: %s", refr->GetName());
#endif  // DEBUG

			proj->desiredTarget = refr->GetHandle();
			return refr->As<RE::Actor>();
		}
	}

	namespace Moving
	{
		// SkyrimSE.exe+74DC20
		float get_proj_speed(RE::Projectile* proj)
		{
			return _generic_foo_<42958, decltype(get_proj_speed)>::eval(proj);
		}

		RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime)
		{
			RE::NiPoint3 ans, eye_pos;
			target->GetLinearVelocity(ans);
			ans *= dtime;
			FenixUtils::Actor__get_eye_pos(target, eye_pos, 3);
			ans += eye_pos;
			return ans;
		}

		bool get_shoot_dir(RE::Projectile* proj, RE::Actor* target, float dtime,
			RE::NiPoint3& ans)
		{
			RE::NiPoint3 target_dir;
			target->GetLinearVelocity(target_dir);
			double target_speed = target_dir.Length();

			double proj_speed = get_proj_speed(proj);

			auto target_pos = get_victim_pos(target, dtime);
			auto strait_dir = target_pos - proj->GetPosition();

			double a = proj_speed * proj_speed - target_speed * target_speed;

			double strait_len = strait_dir.Unitize();
			double c = -(strait_len * strait_len);
			double b;

			if (target_speed > 0.0001) {
				target_dir.Unitize();
				double cos_phi = -target_dir.Dot(strait_dir);
				b = 2 * strait_len * target_speed * cos_phi;
			} else {
				b = 0.0;
			}

			double D = b * b - 4 * a * c;
			if (D < 0)
				return false;

			D = sqrt(D);
			double t1 = (-b + D) / a * 0.5;
			double t2 = (-b - D) / a * 0.5;

			if (t1 <= 0 && t2 <= 0)
				return false;

			double t = t1;
			if (t2 > 0 && t2 < t1)
				t = t2;

			ans = target_dir * (float)target_speed + strait_dir * (float)(strait_len / t);
			return true;
		}

		// constant speed, limited rotation angle
		void change_direction_1(RE::Projectile* proj, float dtime,
			const RE::NiPoint3& final_vel, float param)
		{
			auto get_rotation_speed = []([[maybe_unused]] RE::Projectile* proj,
										  float param) {
				// param1 / 100 = time to rotate at 180
				// 250 350 500 norm
				return 3.1415926f / param;
			};

			auto final_dir = final_vel;
			final_dir.Unitize();
			auto old_dir = proj->linearVelocity;
			old_dir.Unitize();

			float max_angle = get_rotation_speed(proj, param) * dtime;
			float angle = acos(old_dir.Dot(final_dir));
			auto axis = old_dir.UnitCross(final_dir);

			float phi = fmin(max_angle, angle);
			float cos_phi = cos(phi);
			float sin_phi = sin(phi);
			float one_cos_phi = 1 - cos_phi;
			RE::NiMatrix3 R = { { cos_phi + one_cos_phi * axis.x * axis.x,
									axis.x * axis.y * one_cos_phi - axis.z * sin_phi,
									axis.x * axis.z * one_cos_phi + axis.y * sin_phi },
				{ axis.y * axis.x * one_cos_phi + axis.z * sin_phi,
					cos_phi + axis.y * axis.y * one_cos_phi,
					axis.y * axis.z * one_cos_phi - axis.x * sin_phi },
				{ axis.z * axis.x * one_cos_phi - axis.y * sin_phi,
					axis.z * axis.y * one_cos_phi + axis.x * sin_phi,
					cos_phi + axis.z * axis.z * one_cos_phi } };

			proj->linearVelocity = (R * old_dir) * proj->linearVelocity.Length();
		}

		// constant acceleration length
		void change_direction_2(RE::Projectile* proj, [[maybe_unused]] float dtime,
			const RE::NiPoint3& final_vel, float param)
		{
			auto get_acceleration = []([[maybe_unused]] RE::Projectile* proj,
										float param) {
				// param1 / 10 = acceleration vector length
				// 50 100 500
				return param;
			};

			auto V = final_vel;
			auto speed = proj->linearVelocity.Length();
			V.Unitize();
			V *= speed;
			V -= proj->linearVelocity;
			V.Unitize();
			V *= get_acceleration(proj, param);
			speed = get_proj_speed(proj);
			proj->linearVelocity += V;
			float newspeed = proj->linearVelocity.Length();
			proj->linearVelocity *= speed / newspeed;
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

		void change_direction_linVel(RE::Projectile* proj, float dtime)
		{
			RE::NiPoint3 final_vel;
			if (auto target = Targeting::findTarget(proj); target && get_shoot_dir(proj, target, dtime, final_vel)) {
				auto type = Data::get_AutoAimType(proj);
				auto param = Data::get_AutoAimParam(proj);
				switch (type) {
				case AutoAimTypes::ConstSpeed:
					change_direction_1(proj, dtime, final_vel, param);
					break;
				case AutoAimTypes::ConstAccel:
					change_direction_2(proj, dtime, final_vel, param);
					break;
				default:
					break;
				}
			} else {
				AutoAim::Data::disable_AutoAim(proj);
			}
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
		// Make projectile move to the target
		class AutoAimHook
		{
		public:
			static void Hook()
			{
				_Projectile__apply_gravity =
					SKSE::GetTrampoline().write_call<5>(REL::ID(43006).address() + 0x69,
						change_direction);  // SkyrimSE.exe+751309
			}

		private:
			static bool change_direction(RE::Projectile* proj, RE::NiPoint3* dV,
				float dtime)
			{
				bool ans = _Projectile__apply_gravity(proj, dV, dtime);
				if (Data::is_AutoAimType(proj)) {
					Moving::change_direction(proj, dV, dtime);
				}
				return ans;
			}

			static inline REL::Relocation<decltype(change_direction)>
				_Projectile__apply_gravity;
		};

		// Draw debug lines to captured targets
		class CursorDetectedHook
		{
		public:
			static void Hook()
			{
				_Update =
					REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0]))
						.write_vfunc(0xad, Update);
			}

		private:
			static void Update(RE::PlayerCharacter* a, float delta)
			{
				_Update(a, delta);

				if (auto obj = a->GetEquippedObject(false))
					if (auto spel = obj->As<RE::SpellItem>())
						if (auto mgef = FenixUtils::getAVEffectSetting(spel))
							if (auto target = Targeting::Cursor::find_cursor_target(a,
									std::get<float>(
										get_findTarget_data(mgef->data.projectileBase))))
								draw_line(a->GetPosition(), target->GetPosition(), 5, 0);
			}

			static inline REL::Relocation<decltype(Update)> _Update;
		};

		// Draw debug circle of target capturing
		class CursorCircleHook
		{
		public:
			static void Hook()
			{
				_Update =
					REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0]))
						.write_vfunc(0xad, Update);
			}

		private:
			static void Update(RE::PlayerCharacter* a, float delta)
			{
				_Update(a, delta);

				if (auto obj = a->GetEquippedObject(false))
					if (auto spel = obj->As<RE::SpellItem>())
						if (auto mgef = FenixUtils::getAVEffectSetting(spel)) {
							float alpha_max = std::get<float>(
								get_findTarget_data(mgef->data.projectileBase));
							alpha_max = alpha_max / 180.0f * 3.1415926f;

							RE::NiPoint3 origin, caster_dir;
							FenixUtils::Actor__get_eye_pos(a, origin, 2);

							const float circle_dist = 2000;
							caster_dir = FenixUtils::rotate(1, a->data.angle);

							float circle_r = circle_dist * tan(alpha_max);
							RE::NiPoint3 right_dir =
								RE::NiPoint3(0, 0, -1).UnitCross(caster_dir);
							if (right_dir.SqrLength() == 0)
								right_dir = { 1, 0, 0 };
							right_dir *= circle_r;
							RE::NiPoint3 up_dir = right_dir.Cross(caster_dir);

							origin += caster_dir * circle_dist;

							RE::NiPoint3 old = origin + right_dir;
							const int N = 31;
							for (int i = 1; i <= N; i++) {
								float alpha = 2 * 3.1415926f / N * i;

								auto cur_p =
									origin + right_dir * cos(alpha) + up_dir * sin(alpha);

								draw_line(old, cur_p, 5, 0);
								old = cur_p;
							}
						}
			}

			static inline REL::Relocation<decltype(Update)> _Update;
		};
	}

	auto rot_at(RE::NiPoint3 dir)
	{
		ProjectileRot rot;
		auto len = dir.Unitize();
		if (len == 0) {
			rot = { 0, 0 };
		} else {
			float polar_angle = _generic_foo_<68820, float(RE::NiPoint3 * p)>::eval(
				&dir);  // SkyrimSE.exe+c51f70
			rot = { -asin(dir.z), polar_angle };
		}

		return rot;
	}

	auto rot_at(const RE::NiPoint3& from, const RE::NiPoint3& to)
	{
		return rot_at(to - from);
	}

	void onCreated(RE::Projectile* proj, uint32_t key)
	{
		if (auto data = get_onCreate_data(key);
			std::get<bool>(data)) {
			if (auto caster = proj->shooter.get().get()) {
				bool isPlayer = caster->IsPlayerRef();
				auto caster_type = std::get<AutoAimCaster>(data);
				if (caster_type == AutoAimCaster::NPC && !isPlayer ||
					caster_type == AutoAimCaster::Player && isPlayer ||
					caster_type == AutoAimCaster::Both) {
					if (proj->IsBeamProjectile() || proj->IsFlameProjectile()) {
						if (auto target = Targeting::findTarget(proj)) {
							auto dir = rot_at(proj->GetPosition(),
								Moving::get_victim_pos(target, 0));

							_generic_foo_<19362, void(RE::TESObjectREFR * refr,
													 float rot_Z)>::eval(proj, dir.z);
							_generic_foo_<19360, void(RE::TESObjectREFR * refr,
													 float rot_X)>::eval(proj, dir.x);
						}
					}

					if (proj->IsMissileProjectile()) {
						Data::set_AutoAimType(proj, std::get<AutoAimTypes>(data),
							std::get<float>(data));
					}
				}
			}
		}
	}

	void onCreated(RE::Projectile* proj)
	{
		return onCreated(proj, proj->GetBaseObject()->formID);
	}

	void forget() { Data::JsonStorage::clear(); }

	void add_mod(const Json::Value& homing, int hex)
	{
		Data::JsonStorage::init(homing, hex);
	}

	void install()
	{
		Hooks::AutoAimHook::Hook();

		if (Settings::Enable) {
			if (Settings::CursorDetected) {
				Hooks::CursorDetectedHook::Hook();
			}

			if (Settings::CursorCircle) {
				Hooks::CursorCircleHook::Hook();
			}
		}
	}
}
