#include "NewProjectiles.h"
#include "FenixProjectilesAPI.h"

FenixProjsType& get_proj_type(RE::Projectile* proj) { return (FenixProjsType&)(uint32_t&)proj->pad164; }

bool is_CustomPosType(RE::Projectile* proj) { return get_proj_type(proj).isCustomPos(); }
void set_CustomPosType(RE::Projectile* proj) { get_proj_type(proj).setCustomPos(); }

bool is_AutoAimType(RE::Projectile* proj) { return get_proj_type(proj).isAutoAim(); }
AutoAimTypes get_AutoAimType(RE::Projectile* proj) { return get_proj_type(proj).autoAimType(); }
uint32_t get_AutoAimParam(RE::Projectile* proj) { return get_proj_type(proj).get_AutoAimParam(); }
void set_AutoAimType(RE::Projectile* proj, AutoAimData_t type)
{
	get_proj_type(proj).set_AutoAim(type.first);
	get_proj_type(proj).set_AutoAimParam(type.second.param);
	proj->flags |= 1 << 6;  // smth like "no gravity". To updating model rotation.
}

void set_NormalType(RE::Projectile* proj) { get_proj_type(proj).set_NormalType(); }

using HomieType = std::unordered_map<uint32_t, JsonData>;

HomieType ConstSpeed;  // Const Speed
HomieType ConstAccel;  // Const Acceleration

auto findHomie(const HomieType& storage, RE::Projectile* proj)
{
	auto _bproj = proj->GetBaseObject();
	if (auto bproj = _bproj ? _bproj->As<RE::BGSProjectile>() : nullptr) {
		auto formid = bproj->formID;
		return storage.find(formid);
	} else {
		return storage.end();
	}
}

AutoAimData_t is_homie(RE::Projectile* proj)
{
	if (auto found = findHomie(ConstAccel, proj); found != ConstAccel.end()) {
		return { AutoAimTypes::ConstAcc, (*found).second };
	}

	if (auto found = findHomie(ConstSpeed, proj); found != ConstSpeed.end()) {
		return { AutoAimTypes::ConstSpeed, (*found).second };
	}

	JsonData data;
	data.param = 0;
	data.target = AutoAimTarget::Nearest;
	return { AutoAimTypes::Normal, data };
}

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

void init_Launchdata(Projectile__LaunchData& ldata, RE::Actor* a, const RE::NiPoint3& startPos, RE::SpellItem* spel,
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

RE::NiPoint3* get_eye_pos(RE::Actor* me, const RE::NiPoint3& ans, int mb_type)
{
	return _generic_foo_<36755, decltype(get_eye_pos)>::eval(me, ans, mb_type);
}

namespace AutoAim
{
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

	RE::TESObjectREFR* find_cursor_target(RE::TESObjectREFR* _caster)
	{
		if (!_caster->IsPlayerRef())
			return nullptr;

		auto caster = _caster->As<RE::Actor>();

		RE::NiPoint3 ray_start, ray_end;
		
		FenixUtils::Actor__get_eye_pos(caster, &ray_start, 2);

		ray_end = ray_start;
		float reach = 10000;
		if (caster->IsPlayerRef()) {
			ray_end += FenixUtils::rotate(reach, caster->data.angle);
		} else {
			ray_end += (caster->data.location - ray_end) * 0.25f;
			ray_end += FenixUtils::rotateZ(reach, caster->data.angle);
		}

		auto havokWorldScale = RE::bhkWorld::GetWorldScale();
		RE::bhkPickData pick_data;
		pick_data.rayInput.from = ray_start * havokWorldScale;
		pick_data.rayInput.to = ray_end * havokWorldScale;

		uint32_t col_info = 0;
		caster->GetCollisionFilterInfo(col_info);
		pick_data.rayInput.filterInfo =
			(static_cast<uint32_t>(col_info >> 16) << 16) | static_cast<uint32_t>(RE::COL_LAYER::kCharController);

		caster->GetParentCell()->GetbhkWorld()->PickObject(pick_data);

		if (pick_data.rayOutput.HasHit()) {
			if (auto collidable = pick_data.rayOutput.rootCollidable) {
				return RE::TESHavokUtilities::FindCollidableRef(*collidable);
			}
		}
		return nullptr;
	}

	RE::Actor* findTarget(RE::Projectile* proj)
	{
		auto target = proj->desiredTarget.get().get();
		if (target)
			return target->As<RE::Actor>();

		auto caster = proj->shooter.get().get();
		if (!caster)
			return nullptr;

		RE::TESObjectREFR* refr;
		auto target_type = is_homie(proj).second.target;
		switch (target_type) {
		case AutoAimTarget::Nearest:
		case AutoAimTarget::NearestHostile:
			refr = find_nearest_target(caster, proj, target_type == AutoAimTarget::NearestHostile);
			break;
		case AutoAimTarget::AtCursor:
			refr = find_cursor_target(caster);
			break;
		default:
			refr = nullptr;
			break;
		}

		if (!refr)
			return nullptr;

#ifdef DEBUG
		char sMsgBuff[128] = { 0 };
		sprintf_s(sMsgBuff, "Target found: %s", refr->GetName());
		RE::DebugNotification(sMsgBuff);
#endif  // DEBUG

		proj->desiredTarget = refr->GetHandle();
		return refr->As<RE::Actor>();
	}

	RE::NiPoint3 get_victim_pos(RE::Actor* target, float dtime)
	{
		RE::NiPoint3 ans, eye_pos;
		target->GetLinearVelocity(ans);
		ans *= dtime;
		get_eye_pos(target, eye_pos, 2);
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
			// param / 100 = time to rotate at 180
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
			// param / 10 = acceleration vector length
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

			// update rotation
			_generic_foo_<43010, void(RE::TESObjectREFR * a, const RE::NiPoint3& P)>::eval(proj,
				proj->GetPosition() + proj->linearVelocity);
		} else {
			set_NormalType(proj);
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

AutoAimTarget read_json_(const std::string& s)
{
	if (s == "cursor")
		return AutoAimTarget::AtCursor;
	if (s == "hostile")
		return AutoAimTarget::NearestHostile;
	if (s == "nearest")
		return AutoAimTarget::Nearest;

	return AutoAimTarget::Nearest;
}

void read_json_(HomieType& storage, const Json::Value& items, int hex)
{
	for (int i = 0; i < (int)items.size(); i++) {
		const auto& item = items[i];
		uint32_t formid = std::stoul(item[0].asString(), nullptr, 16);

		JsonData data;
		data.param = item[1].asUInt();
		data.target = read_json_(item[2].asString());
		storage.insert({ hex | formid, data });
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

		const auto& mod_data = json_root[mod_name];
		for (auto& type : mod_data.getMemberNames()) {
			if (type == "ConstSpeed")
				read_json_(ConstSpeed, mod_data[type], hex);
			else if (type == "ConstAccel")
				read_json_(ConstAccel, mod_data[type], hex);
		}
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
