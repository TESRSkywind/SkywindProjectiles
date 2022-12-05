#include "NewProjectiles.h"
#include "FenixProjectilesAPI.h"

FenixProjsRuntimeData& get_proj_type(RE::Projectile* proj) { return (FenixProjsRuntimeData&)(uint32_t&)proj->pad164; }

bool is_CustomPosType(RE::Projectile* proj) { return get_proj_type(proj).isCustomPos(); }
void set_CustomPosType(RE::Projectile* proj) { get_proj_type(proj).setCustomPos(); }

void set_NormalType(RE::Projectile* proj) { get_proj_type(proj).set_NormalType(); }

struct Projectile__LaunchData
{
	void* vftable_LaunchData_0;
	RE::NiPoint3 startPos;
	RE::NiPoint3 Point_14;
	RE::BGSProjectile* projectile;
	RE::TESObjectREFR* source;
	RE::CombatController* combatController;
	RE::TESObjectWEAP* weap;
	RE::TESAmmo* overwriteAmmo;
	float rotationZ;
	float rotationX;
	void* field_50;
	RE::TESObjectREFR* target;
	std::pair<float, float> drawn_time;
	RE::TESObjectCELL* cell;
	RE::MagicItem* CastItem;
	RE::MagicSystem::CastingSource castingSource;
	RE::EnchantmentItem* ammoEnchantment;
	RE::AlchemyItem* poison;
	uint32_t area;
	float field_94;
	float scale;
	char field_9C;
	char field_9D;
	char field_9E;
	char field_9F;
	char field_A0;
	char field_A1;
	char field_A2;
	char field_A3;
	char gapA4[4];
};
static_assert(sizeof(Projectile__LaunchData) == 0xA8);

void init_Launchdata(Projectile__LaunchData& ldata, RE::Actor* a, const RE::NiPoint3& startPos, RE::MagicItem* spel,
	float rotationZ, float rotationX, uint32_t area)
{
	auto mgef = FenixUtils::getAVEffectSetting(spel);

	ldata.vftable_LaunchData_0 = nullptr;  // TODO: mb used
	ldata.startPos = startPos;
	ldata.Point_14 = { 0.0f, 0.0f, 0.0f };
	ldata.projectile = mgef->data.projectileBase;
	ldata.source = a;
	ldata.combatController = a->combatController;
	ldata.weap = nullptr;
	ldata.overwriteAmmo = nullptr;
	ldata.rotationZ = rotationZ;
	ldata.rotationX = rotationX;
	ldata.field_50 = nullptr;
	ldata.target = nullptr;
	ldata.drawn_time.first = 0.0f;
	ldata.drawn_time.second = 0.0f;
	ldata.cell = a->GetParentCell();
	ldata.CastItem = spel;
	ldata.castingSource = RE::MagicSystem::CastingSource::kOther;
	ldata.ammoEnchantment = nullptr;
	ldata.poison = nullptr;
	ldata.area = area;
	ldata.field_94 = 1.0f;
	ldata.scale = 1.0f;
	*(uint32_t*)&ldata.field_9C = 0x10000;
	ldata.field_A0 = 0;
	ldata.field_A1 = 0;
	ldata.field_A2 = 0;
}

uint32_t* CreateProjectile_14074B170(uint32_t* a1, Projectile__LaunchData* LaunchData)
{
	return _generic_foo_<42928, decltype(CreateProjectile_14074B170)>::eval(a1, LaunchData);
}

RE::Effect* GetCostiestEffect(RE::MagicItem* spel, RE::MagicSystem::Delivery type, char ignorearea)
{
	return _generic_foo_<11216, decltype(GetCostiestEffect)>::eval(spel, type, ignorearea);
}

uint32_t cast(RE::MagicCaster* a, RE::Actor* caster, const RE::NiPoint3& start_pos, const ProjectileRot rot)
{
	Projectile__LaunchData ldata;
	auto eff = GetCostiestEffect(a->currentSpell, RE::MagicSystem::Delivery::kAimed, false);

	init_Launchdata(ldata, caster, start_pos, a->currentSpell, rot.z, rot.x, eff ? eff->GetArea() : 0);

	uint32_t handle;
	CreateProjectile_14074B170(&handle, &ldata);

	return handle;
}

uint32_t cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	Projectile__LaunchData ldata;
	auto eff = GetCostiestEffect(spel, RE::MagicSystem::Delivery::kAimed, false);
	
	init_Launchdata(ldata, caster, start_pos, spel, rot.z, rot.x, eff ? eff->GetArea() : 0);

	uint32_t handle;
	CreateProjectile_14074B170(&handle, &ldata);

	return handle;
}

uint32_t cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	auto handle = cast(caster, spel, start_pos, rot);

	RE::TESObjectREFRPtr _refr;
	RE::LookupReferenceByHandle(handle, _refr);
	if (auto proj = _refr.get() ? _refr.get()->As<RE::Projectile>() : nullptr) {
		set_CustomPosType(proj);
	}

	return handle;
}

namespace Sounds
{

	RE::BGSSoundDescriptorForm* EffectSetting__get_sndr(RE::EffectSetting* a1, RE::MagicSystem::SoundID sid)
	{
		return _generic_foo_<11001, decltype(EffectSetting__get_sndr)>::eval(a1, sid);
	}

	void PlaySound_func3_140BEDB10(RE::BSSoundHandle* a1, RE::NiAVObject* source_node)
	{
		return _generic_foo_<66375, decltype(PlaySound_func3_140BEDB10)>::eval(a1, source_node);
	}

	char set_sound_position(RE::BSSoundHandle* shandle, float x, float y, float z)
	{
		return _generic_foo_<66370, decltype(set_sound_position)>::eval(shandle, x, y, z);
	}

	void play_cast_sound(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos)
	{
		if (auto root = caster->Get3D2()) {
			RE::BSSoundHandle shandle;
			RE::BGSSoundDescriptorForm* sndr = nullptr;
			auto sid = RE::MagicSystem::SoundID::kRelease;
			if (auto eff = FenixUtils::getAVEffectSetting(spel)) {
				sndr = EffectSetting__get_sndr(eff, sid);
			}
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(shandle, sndr, 0);
			PlaySound_func3_140BEDB10(&shandle, root);

			set_sound_position(&shandle, start_pos.x, start_pos.y, start_pos.z);
			shandle.Play();
		}
	}
}

uint32_t cast_CustomPos_withsound(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	auto handle = cast_CustomPos(caster, spel, start_pos, rot);

	Sounds::play_cast_sound(caster, spel, start_pos);

	return handle;
}

namespace AutoAim
{
	using HomieType = std::unordered_map<uint32_t, JsonData>;

	HomieType ConstSpeed;  // Const Speed
	HomieType ConstAccel;  // Const Acceleration

	bool is_AutoAimType(RE::Projectile* proj) { return get_proj_type(proj).isAutoAim(); }
	AutoAimTypes get_AutoAimType(RE::Projectile* proj) { return get_proj_type(proj).autoAimType(); }
	uint32_t get_AutoAimParam(RE::Projectile* proj) { return get_proj_type(proj).get_AutoAimParam(); }
	void set_AutoAimType(RE::Projectile* proj, AutoAimData_t type)
	{
		get_proj_type(proj).set_AutoAim(type.first);
		get_proj_type(proj).set_AutoAimParam(type.second.param1);
	}
	
	auto findHomie(const HomieType& storage, RE::TESBoundObject* _bproj)
	{
		if (auto bproj = _bproj ? _bproj->As<RE::BGSProjectile>() : nullptr) {
			auto formid = bproj->formID;
			return storage.find(formid);
		} else {
			return storage.end();
		}
	}

	AutoAimData_t is_homie(RE::TESBoundObject* _bproj)
	{
		if (auto found = findHomie(ConstAccel, _bproj); found != ConstAccel.end()) {
			return { AutoAimTypes::ConstAcc, (*found).second };
		}

		if (auto found = findHomie(ConstSpeed, _bproj); found != ConstSpeed.end()) {
			return { AutoAimTypes::ConstSpeed, (*found).second };
		}

		JsonData data;
		data.param1 = 0;
		data.param2 = 0;
		data.target = AutoAimTarget::Nearest;
		data.caster = AutoAimCaster::Both;
		return { AutoAimTypes::Normal, data };
	}

	bool is_hostile(RE::TESObjectREFR* refr, RE::TESObjectREFR* _caster)
	{
		auto target = refr->As<RE::Actor>();
		auto caster = _caster->As<RE::Actor>();
		if (!target || !caster)
			return false;

		return target->currentCombatTarget.get().get() == caster;
	}

	RE::TESObjectREFR* find_nearest_target(RE::TESObjectREFR* caster, RE::Projectile* proj, bool onlyHostile)
	{
		float mindist2 = 1.0E15f;
		RE::TESObjectREFR* refr = nullptr;
		RE::TES::GetSingleton()->ForEachReference([=, &mindist2, &refr](RE::TESObjectREFR& _refr) {
			if (!_refr.IsDisabled() && !_refr.IsDead() && _refr.GetFormType() == RE::FormType::ActorCharacter &&
				_refr.formID != caster->formID && (!onlyHostile || is_hostile(&_refr, caster))) {
				float curdist = proj->GetPosition().GetSquaredDistance(_refr.GetPosition());
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

	static bool is_anglebetween_less(const RE::NiPoint3& A, const RE::NiPoint3& B1, const RE::NiPoint3& B2, float angle_deg)
	{
		auto AB1 = B1 - A;
		auto AB2 = B2 - A;
		AB1.Unitize();
		AB2.Unitize();
		return acos(AB1.Dot(AB2)) < angle_deg / 180.0f * 3.1415926f;
	}

	static bool is_near_to_cursor(RE::Actor* caster, RE::Actor* target, float angle)
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

	static LineOfSightLocation IsActorInLineOfSight(RE::Actor* caster, RE::Actor* target, float viewCone = 100)
	{
		return _generic_foo_<36752, decltype(IsActorInLineOfSight)>::eval(caster, target, viewCone);
	}

	RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster, float angle)
	{
		if (!_caster->IsPlayerRef())
			return nullptr;

		auto caster = _caster->As<RE::Actor>();
		std::vector<std::pair<RE::Actor*, float>> targets;

		RE::TES::GetSingleton()->ForEachReference([caster, &targets, angle](RE::TESObjectREFR& _refr) {
			if (auto refr = _refr.As<RE::Actor>(); refr && !_refr.IsDisabled() && !_refr.IsDead() &&
												   _refr.GetFormType() == RE::FormType::ActorCharacter &&
												   _refr.formID != caster->formID && is_near_to_cursor(caster, refr, angle)) {
				if (IsActorInLineOfSight(caster, refr) != LineOfSightLocation::kNone) {
					targets.push_back({ refr, caster->GetPosition().GetSquaredDistance(refr->GetPosition()) });
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});

		if (!targets.size())
			return nullptr;

		return (*std::min_element(targets.begin(), targets.end(),
			 [](const std::pair<RE::Actor*, float>& a, const std::pair<RE::Actor*, float>& b) { return a.second < b.second; }))
			.first;
	}

	RE::Actor* findTarget(RE::Projectile* proj)
	{
		auto target = proj->desiredTarget.get().get();
		if (target)
			return target->As<RE::Actor>();

		auto caster = proj->shooter.get().get();
		if (!caster)
			return nullptr;

		if (auto caster_npc = caster->As<RE::Actor>(); caster_npc && !caster_npc->IsPlayerRef()) {
			return caster_npc->currentCombatTarget.get().get();
		}

		RE::TESObjectREFR* refr;
		auto homie_data = is_homie(proj->GetBaseObject()).second;
		auto target_type = homie_data.target;
		switch (target_type) {
		case AutoAimTarget::Nearest:
		case AutoAimTarget::NearestHostile:
			refr = find_nearest_target(caster, proj, target_type == AutoAimTarget::NearestHostile);
			break;
		case AutoAimTarget::AtCursor:
			refr = find_cursor_target(caster, static_cast<float>(homie_data.param2));
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

	RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime)
	{
		RE::NiPoint3 ans, eye_pos;
		target->GetLinearVelocity(ans);
		ans *= dtime;
		FenixUtils::Actor__get_eye_pos(target, eye_pos, 3);
		ans += eye_pos;
		return ans;
	}
	
	float get_proj_speed(RE::Projectile* proj) {
		return proj->GetBaseObject()->As<RE::BGSProjectile>()->data.speed * proj->GetPowerSpeedMult() * proj->speedMult;
	}

	bool get_shoot_dir(RE::Projectile* proj, RE::Actor* target, float dtime, RE::NiPoint3& ans) {
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
	void change_direction_1(RE::Projectile* proj, float dtime, const RE::NiPoint3& final_vel, uint32_t param)
	{
		auto get_rotation_speed = []([[maybe_unused]] RE::Projectile* proj, uint32_t param) {
			// param1 / 100 = time to rotate at 180
			// 250 350 500 norm
			return 3.14159265358979323f / (static_cast<float>(param) * 0.01f);
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
		RE::NiMatrix3 R = { { cos_phi + one_cos_phi * axis.x * axis.x, axis.x * axis.y * one_cos_phi - axis.z * sin_phi,
								axis.x * axis.z * one_cos_phi + axis.y * sin_phi },
			{ axis.y * axis.x * one_cos_phi + axis.z * sin_phi, cos_phi + axis.y * axis.y * one_cos_phi,
				axis.y * axis.z * one_cos_phi - axis.x * sin_phi },
			{ axis.z * axis.x * one_cos_phi - axis.y * sin_phi, axis.z * axis.y * one_cos_phi + axis.x * sin_phi,
				cos_phi + axis.z * axis.z * one_cos_phi } };

		proj->linearVelocity = (R * old_dir) * proj->linearVelocity.Length();
	}

	// constant acceleration length
	void change_direction_2(RE::Projectile* proj, [[maybe_unused]] float dtime, const RE::NiPoint3& final_vel, uint32_t param)
	{
		auto get_acceleration = []([[maybe_unused]] RE::Projectile* proj, uint32_t param) {
			// param1 / 10 = acceleration vector length
			// 50 100 500
			return static_cast<float>(param) * 0.1f;
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

	void change_direction(RE::Projectile* proj, RE::NiPoint3*, [[maybe_unused]] float dtime) {
		if (auto target = findTarget(proj)) {

			RE::NiPoint3 final_vel;
			if (!get_shoot_dir(proj, target, dtime, final_vel))
				return set_NormalType(proj);

			auto type = get_AutoAimType(proj);
			auto param = get_AutoAimParam(proj);
			switch (type) {
			case AutoAimTypes::ConstSpeed:
				change_direction_1(proj, dtime, final_vel, param);
				break;
			case AutoAimTypes::ConstAcc:
				change_direction_2(proj, dtime, final_vel, param);
				break;
			default:
				break;
			}

#ifdef DEBUG
			{
				auto proj_dir = proj->linearVelocity;
				proj_dir.Unitize();
				draw_line<Colors::RED>(proj->GetPosition(), proj->GetPosition() + proj_dir);
			}
#endif  // DEBUG

			// set node rotation
			RE::NiPoint3 proj_dir = proj->linearVelocity;
			proj_dir.Unitize();

			proj->data.angle.x = asin(proj_dir.z);
			proj->data.angle.z = atan2(proj_dir.x, proj_dir.y);

			if (proj_dir.x < 0.0) {
				proj->data.angle.z += 3.1415926f;
			}

			if (proj->data.angle.z < 0.0) {
				proj->data.angle.z += 3.1415926f;
			}

			SetRotationMatrix(proj->Get3D2()->local.rotate, proj_dir.x, proj_dir.y, proj_dir.z);
		} else {
			set_NormalType(proj);
		}
	}
}

namespace ManyProjs
{
	using HomieType = std::unordered_map<uint32_t, JsonData>;

	HomieType ManyProjsStorage;

	auto findBproj(RE::TESBoundObject* _bproj)
	{
		if (auto bproj = _bproj ? _bproj->As<RE::BGSProjectile>() : nullptr) {
			auto formid = bproj->formID;
			return ManyProjsStorage.find(formid);
		} else {
			return ManyProjsStorage.end();
		}
	}

	bool is_ManyCast(RE::TESBoundObject* _bproj) { return findBproj(_bproj) != ManyProjsStorage.end(); }

	auto get_data(RE::TESBoundObject* _bproj)
	{
		if (auto found = findBproj(_bproj); found != ManyProjsStorage.end()) {
			return (*found).second;
		}

		JsonData data;
		data.shape = Shape::Single;
		data.launch = LaunchType::Parallel;
		data.count = 1;
		data.size = 0;
		return data;
	}

	struct Plane
	{
		RE::NiPoint3 startPos, right_dir, up_dir;
	};

	struct ShapeParams
	{
		float size;
		uint32_t count;
	};

	using callback_t = void(const RE::NiPoint3& startPos, const ProjectileRot rot);
	const std::array<
		std::function<void(std::function<callback_t> cast_here, const ShapeParams& shape_params,
			const Plane& plane, const ProjectileRot& rot)>,
		5>
		MultiCasters = {

			// Single
			[](std::function<callback_t> cast_here, ShapeParams,
				const Plane& plane,
				const ProjectileRot rot) { cast_here(plane.startPos, rot); },

			// HorisontalLine
			[](std::function<callback_t> cast_here, ShapeParams shape_params,
				const Plane& plane, const ProjectileRot rot) {
				if (shape_params.count == 1) {
					cast_here(plane.startPos, rot);
					return;
				}

				auto from = plane.startPos - plane.right_dir * (shape_params.size * 0.5f);
				float d = shape_params.size / (shape_params.count - 1);
				for (uint32_t i = 0; i < shape_params.count; i++) {
					cast_here(from, rot);
					from += plane.right_dir * d;
				}
			},

			// VerticalLine
			[](std::function<callback_t> cast_here, ShapeParams shape_params,
				const Plane& plane, const ProjectileRot rot) {
				if (shape_params.count == 1) {
					cast_here(plane.startPos, rot);
					return;
				}

				auto from = plane.startPos - plane.up_dir * (shape_params.size * 0.5f);
				float d = shape_params.size / (shape_params.count - 1);
				for (uint32_t i = 0; i < shape_params.count; i++) {
					cast_here(from, rot);
					from += plane.up_dir * d;
				}
			},

			// Circle
			[](std::function<callback_t> cast_here, ShapeParams shape_params,
				const Plane& plane, const ProjectileRot rot) {
				for (uint32_t i = 0; i < shape_params.count; i++) {
					float alpha = 2 * 3.1415926f / shape_params.count * i;

					auto cur_p = plane.startPos +
			                     plane.right_dir * cos(alpha) * shape_params.size +
			                     plane.up_dir * sin(alpha) * shape_params.size;

					cast_here(cur_p, rot);
				}
			},

			// FillSquare
			[](std::function<callback_t> cast_here, ShapeParams shape_params,
				const Plane& plane, const ProjectileRot rot) {
				uint32_t h = (uint32_t)ceil(sqrt(shape_params.count));

				if (h == 1) {
					MultiCasters[(uint32_t)Shape::HorisontalLine](cast_here, shape_params,
						plane, rot);
					return;
				}

				auto from = plane.startPos + plane.up_dir * (shape_params.size * 0.5f);
				float d = shape_params.size / (h - 1);
				
				ShapeParams params = {shape_params.size, h};
				Plane cur_plane = plane;
				while (shape_params.count >= h) {
					cur_plane.startPos = from;
					MultiCasters[(uint32_t)Shape::HorisontalLine](cast_here, params,
						cur_plane, rot);
					from -= plane.up_dir * d;
					shape_params.count -= h;
				}

				if (shape_params.count > 0) {
					cur_plane.startPos = from;
					params.count = shape_params.count;
					MultiCasters[(uint32_t)Shape::HorisontalLine](cast_here, params,
						cur_plane, rot);
				}
			}
		};

	void onManyCasted(RE::MagicCaster* magic_caster, RE::Actor* caster,
		RE::NiPoint3* startPos, const ProjectileRot rot)
	{
		if (auto spel = magic_caster->currentSpell ?
		                    magic_caster->currentSpell->As<RE::SpellItem>() :
		                    nullptr)
			if (auto mgef = FenixUtils::getAVEffectSetting(spel))
				if (auto bproj = mgef->data.projectileBase) {
					auto data = get_data(bproj);

					auto cast_here = [=](const RE::NiPoint3& startPos,
										 const ProjectileRot rot) {
						cast_CustomPos_withsound(caster, spel, startPos, rot);
					};

					float size = static_cast<float>(data.size);
					RE::NiPoint3 caster_dir;

					caster_dir = FenixUtils::rotate(1, caster->data.angle);

					RE::NiPoint3 right_dir = RE::NiPoint3(0, 0, -1).UnitCross(caster_dir);
					if (right_dir.SqrLength() == 0)
						right_dir = {1, 0, 0};
					RE::NiPoint3 up_dir = right_dir.Cross(caster_dir);

					MultiCasters[(uint32_t)data.shape](cast_here, { size, data.count },
						{ *startPos, right_dir, up_dir }, rot);
				}
	}
}

#include "json/json.h"

int get_mod_index(const std::string_view& name)
{
	auto esp = RE::TESDataHandler::GetSingleton()->LookupModByName(name);
	if (!esp)
		return -1;

	return !esp->IsLight() ? esp->compileIndex << 24 : (0xFE000 | esp->smallFileCompileIndex) << 12;
}

namespace AutoAim
{
	AutoAimTarget read_json_target(const std::string& s)
	{
		if (s == "Cursor")
			return AutoAimTarget::AtCursor;
		if (s == "Hostile")
			return AutoAimTarget::NearestHostile;
		if (s == "Nearest")
			return AutoAimTarget::Nearest;

		return AutoAimTarget::Nearest;
	}

	AutoAimCaster read_json_caster(const std::string& s)
	{
		if (s == "Both")
			return AutoAimCaster::Both;
		if (s == "Player")
			return AutoAimCaster::Player;
		if (s == "NPC")
			return AutoAimCaster::NPC;

		return AutoAimCaster::Both;
	}

	enum class HomingType
	{
		ConstAccel,
		ConstSpeed
	};

	HomingType read_json_type(const std::string& s) {
		if (s == "ConstAccel")
			return HomingType::ConstAccel;
		if (s == "ConstSpeed")
			return HomingType::ConstSpeed;

		return HomingType::ConstAccel;
	}

	void read_json_(const Json::Value& item, uint32_t formid)
	{
		JsonData data;
		if (item.isMember("caster"))
			data.caster = read_json_caster(item["caster"].asString());
		else
			data.caster = AutoAimCaster::Both;

		data.target = read_json_target(item["target"].asString());
		if (data.target == AutoAimTarget::AtCursor)
			data.param2 = item["cursorAngle"].asUInt();

		auto type = read_json_type(item["type"].asString());
		switch (type) {
		case AutoAim::HomingType::ConstAccel:
			data.param1 = item["acceleration"].asUInt();
			ConstAccel.insert({ formid, data });
			break;
		case AutoAim::HomingType::ConstSpeed:
			data.param1 = item["rotationTime"].asUInt();
			ConstSpeed.insert({ formid, data });
			break;
		default:
			break;
		}
	}

	void read_json(const Json::Value& homing, int hex) {
		for (auto& formid : homing.getMemberNames()) {
			read_json_(homing[formid], hex | std::stoul(formid, nullptr, 16));
		}
	}
}

namespace ManyProjs
{
	Shape read_json_shape(const std::string& s) {
		if (s == "Single")
			return Shape::Single;
		if (s == "HorisontalLine")
			return Shape::HorisontalLine;
		if (s == "VerticalLine")
			return Shape::VerticalLine;
		if (s == "Circle")
			return Shape::Circle;
		if (s == "FillSquare")
			return Shape::FillSquare;
		if (s == "FillSphere")
			return Shape::FillSphere;
		
		return Shape::Single;
	}
	
	LaunchType read_json_launch(const std::string& s)
	{
		if (s == "Parallel")
			return LaunchType::Parallel;
		if (s == "ToSight")
			return LaunchType::ToSight;
		
		return LaunchType::Parallel;
	}

	void read_json_(const Json::Value& item, uint32_t formid)
	{
		JsonData data;
		data.shape = read_json_shape(item["shape"].asString());
		switch (data.shape) {
		case Shape::HorisontalLine:
		case Shape::VerticalLine:
		case Shape::Circle:
		case Shape::FillSquare:
			data.size = item["size"].asUInt();
			data.count = item["count"].asUInt();
			if (item.isMember("rotation"))
				data.launch = read_json_launch(item["rotation"].asString());
			else
				data.launch = LaunchType::Parallel;
			break;
		
		default:
			break;
		}
		
		ManyProjsStorage.insert({ formid, data });
	}

	void read_json(const Json::Value& multicast, int hex)
	{
		for (auto& formid : multicast.getMemberNames()) {
			read_json_(multicast[formid], hex | std::stoul(formid, nullptr, 16));
		}
	}
}

void read_json_mod(const Json::Value& mod_data, int hex) {
	for (auto& type : mod_data.getMemberNames()) {
		if (type == "Homing")
			AutoAim::read_json(mod_data[type], hex);
		else if (type == "Multicast")
			ManyProjs::read_json(mod_data[type], hex);
	}
}

void read_json()
{
	Json::Value json_root;
	std::ifstream ifs;
	ifs.open("Data/HomingProjectiles/HomieSpells.json");
	ifs >> json_root;
	ifs.close();

	for (auto& mod_name : json_root.getMemberNames()) {
		int hex = get_mod_index(mod_name);
		if (hex == -1) {
			logger::warn("mod {} not found", mod_name);
			continue;
		}

		read_json_mod(json_root[mod_name], hex);
	}
}

extern "C" 
{
	DLLEXPORT void FenixProjectilesAPI_set_NormalType(RE::Projectile* proj) { return set_NormalType(proj); }

	DLLEXPORT void FenixProjectilesAPI_set_CustomPosType(RE::Projectile* proj) { return set_CustomPosType(proj); }

	DLLEXPORT uint32_t FenixProjectilesAPI_cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast(caster, spel, start_pos, rot);
	}

	DLLEXPORT uint32_t FenixProjectilesAPI_cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast_CustomPos(caster, spel, start_pos, rot);
	}

	DLLEXPORT uint32_t FenixProjectilesAPI_cast_CustomPos_withsound(RE::Actor* caster, RE::SpellItem* spel,
		const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		return cast_CustomPos_withsound(caster, spel, start_pos, rot);
	}
}
