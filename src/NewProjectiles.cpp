#include "NewProjectiles.h"
#include "FenixProjectilesAPI.h"
#include "RuntimeData.h"
#include "AutoAim.h"
#include "Multicast.h"
#include "Emittors.h"

namespace Sounds
{
	RE::BGSSoundDescriptorForm* EffectSetting__get_sndr(RE::EffectSetting* a1,
		RE::MagicSystem::SoundID sid)
	{
		return _generic_foo_<11001, decltype(EffectSetting__get_sndr)>::eval(a1, sid);
	}

	void PlaySound_func3_140BEDB10(RE::BSSoundHandle* a1, RE::NiAVObject* source_node)
	{
		return _generic_foo_<66375, decltype(PlaySound_func3_140BEDB10)>::eval(a1,
			source_node);
	}

	char set_sound_position(RE::BSSoundHandle* shandle, float x, float y, float z)
	{
		return _generic_foo_<66370, decltype(set_sound_position)>::eval(shandle, x, y, z);
	}

	void play_cast_sound(RE::TESObjectREFR* caster, RE::SpellItem* spel,
		const RE::NiPoint3& start_pos)
	{
		if (auto root = caster->Get3D2()) {
			RE::BSSoundHandle shandle;
			RE::BGSSoundDescriptorForm* sndr = nullptr;
			auto sid = RE::MagicSystem::SoundID::kRelease;
			if (auto eff = FenixUtils::getAVEffectSetting(spel)) {
				sndr = EffectSetting__get_sndr(eff, sid);
			}
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(shandle,
				sndr, 0);
			PlaySound_func3_140BEDB10(&shandle, root);

			set_sound_position(&shandle, start_pos.x, start_pos.y, start_pos.z);
			shandle.Play();
		}
	}
}

namespace Impl
{
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

	void init_Launchdata(Projectile__LaunchData& ldata, RE::Actor* a,
		const RE::NiPoint3& startPos, RE::MagicItem* spel, float rotationZ,
		float rotationX, uint32_t area)
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
		return _generic_foo_<42928, decltype(CreateProjectile_14074B170)>::eval(a1,
			LaunchData);
	}

	RE::Effect* GetCostiestEffect(RE::MagicItem* spel, RE::MagicSystem::Delivery type,
		char ignorearea)
	{
		return _generic_foo_<11216, decltype(GetCostiestEffect)>::eval(spel, type,
			ignorearea);
	}

	// `caster` launches `spell`-defined projectile from the given `position`
	uint32_t cast(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos,
		const ProjectileRot& rot)
	{
		Projectile__LaunchData ldata;
		auto eff = GetCostiestEffect(spel, RE::MagicSystem::Delivery::kAimed, false);

		init_Launchdata(ldata, caster, start_pos, spel, rot.z, rot.x,
			eff ? eff->GetArea() : 0);

		uint32_t handle;
		CreateProjectile_14074B170(&handle, &ldata);

		return handle;
	}

	uint32_t cast1(RE::MagicCaster* a, RE::Actor* caster, const RE::NiPoint3& start_pos,
		const ProjectileRot rot)
	{
		Projectile__LaunchData ldata;
		auto eff =
			GetCostiestEffect(a->currentSpell, RE::MagicSystem::Delivery::kAimed, false);

		init_Launchdata(ldata, caster, start_pos, a->currentSpell, rot.z, rot.x,
			eff ? eff->GetArea() : 0);

		uint32_t handle;
		CreateProjectile_14074B170(&handle, &ldata);

		return handle;
	}
}
using Impl::cast;
using Sounds::play_cast_sound;

uint32_t cast_CustomPos(RE::Actor* caster, RE::SpellItem* spel,
	const RE::NiPoint3& start_pos, const ProjectileRot& rot, bool withSound)
{
	auto handle = cast(caster, spel, start_pos, rot);

	RE::TESObjectREFRPtr _refr;
	RE::LookupReferenceByHandle(handle, _refr);
	auto proj = _refr.get() ? _refr.get()->As<RE::Projectile>() : nullptr;
	if (proj)
	{
		set_CustomPosType(proj);
	}

	if (proj && withSound) {
		play_cast_sound(proj, spel, start_pos);
	}

	return handle;
}

uint32_t cast_CustomPos_withsound(RE::Actor* caster, RE::SpellItem* spel, const RE::NiPoint3& start_pos, const ProjectileRot& rot)
{
	return cast_CustomPos(caster, spel, start_pos, rot, true);
}

uint32_t get_formid_fromhex(const std::string& name) {
	return std::stoul(name, nullptr, 16);
}

#include "json/json.h"
void read_json_mod(const Json::Value& mod_data, int hex) {
	for (auto& type : mod_data.getMemberNames()) {
		if (type == "Homing")
			AutoAim::add_mod(mod_data[type], hex);
		else if (type == "Multicast")
			ManyProjs::add_mod(mod_data[type], hex);
		else if (type == "Emitters")
			Emitters::add_mod(mod_data[type], hex);
	}
}

void read_json()
{
	Json::Value json_root;
	std::ifstream ifs;
	ifs.open("Data/HomingProjectiles/HomieSpells.json");
	ifs >> json_root;
	ifs.close();

	JsonUtils::FormIDsMap::init(json_root);
	Emitters::init_global(json_root);

	for (auto& mod_name : json_root.getMemberNames()) {
		if (mod_name == "FormIDs" || mod_name == "Emitters")
			continue;

		int hex = JsonUtils::get_mod_index(mod_name);
		if (hex == -1) {
			logger::warn("mod {} not found", mod_name);
			continue;
		}

		read_json_mod(json_root[mod_name], hex);
	}
}

void reset_json() {
	AutoAim::forget();
	ManyProjs::forget();
	Emitters::forget();

	read_json();
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
