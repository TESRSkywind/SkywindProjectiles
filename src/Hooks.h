#pragma once
#include "FenixProjectilesAPI.h"
#include "NewProjectiles.h"

class PaddingsProjectileHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_TESForm__SetInitedFormFlag_140194B90 =
			trmpl.write_call<5>(REL::ID(42920).address() + 0x392, Ctor);  // SkyrimSE.exe+74ACE2
		_TESObjectREFR__ReadFromSaveGame_140286FD0 =
			trmpl.write_call<5>(REL::ID(42953).address() + 0x4b, LoadGame);  // SkyrimSE.exe+74D28B
	}

private:
	static void Ctor(RE::Projectile* proj, char a2)
	{
		_TESForm__SetInitedFormFlag_140194B90(proj, a2);
		set_NormalType(proj);
	}

	static void __fastcall LoadGame(RE::Projectile* proj, RE::BGSLoadGameBuffer* buf)
	{
		_TESObjectREFR__ReadFromSaveGame_140286FD0(proj, buf);
		set_NormalType(proj);
	}

	static inline REL::Relocation<decltype(Ctor)> _TESForm__SetInitedFormFlag_140194B90;
	static inline REL::Relocation<decltype(LoadGame)> _TESObjectREFR__ReadFromSaveGame_140286FD0;
};

class InitStartPosHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_TESObjectREFR__SetPosition_140296910 =
			trmpl.write_call<5>(REL::ID(43030).address() + 0x215, InitStartPos);          // SkyrimSE.exe+754A35
		_mb_autoaim = trmpl.write_call<5>(REL::ID(43009).address() + 0x201, mb_autoaim);  // SkyrimSE.exe+7518E1
	}

private:
	static void InitStartPos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_CustomPosType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static void mb_autoaim(RE::PlayerCharacter* player, RE::Projectile* proj, RE::NiAVObject* player_bone, float* dir1,
		float* dir2, RE::NiPoint3* a7, float a8, float a9)
	{
		if (!is_CustomPosType(proj)) {
			_mb_autoaim(player, proj, player_bone, dir1, dir2, a7, a8, a9);
		}
	}

	static inline REL::Relocation<decltype(InitStartPos)> _TESObjectREFR__SetPosition_140296910;
	static inline REL::Relocation<decltype(mb_autoaim)> _mb_autoaim;
};

class MultipleBeamsHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_RefHandle__get = trmpl.write_call<5>(REL::ID(42928).address() + 0x117, NewBeam);  // SkyrimSE.exe+74B287

		{
			// SkyrimSE.exe+733F93
			uintptr_t ret_addr = REL::ID(42586).address() + 0x2d3;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func_addr, uintptr_t ret_addr)
				{
					Xbyak::Label nocancel;

					// rsi  = proj
					// xmm0 -- xmm2 = node pos
					mov(r9, rsi);
					mov(rax, func_addr);
					call(rax);
					mov(rax, ret_addr);
					jmp(rax);
				}
			} xbyakCode{ uintptr_t(update_node_pos), ret_addr };

			FenixUtils::add_trampoline<5, 42586, 0x2c1>(&xbyakCode);  // SkyrimSE.exe+621285
		}

		_TESObjectREFR__SetPosition_140296910 =
			trmpl.write_call<5>(REL::ID(42586).address() + 0x2db, UpdatePos);  // SkyrimSE.exe+733F9B
		_j_TESObjectREFR__SetRotationZ_14074FEE0 =
			trmpl.write_call<5>(REL::ID(42586).address() + 0x1ba, UpdateRotZ);                          // SkyrimSE.exe+733E7A
		_Projectile__SetRotationX = trmpl.write_call<5>(REL::ID(42586).address() + 0x1cd, UpdateRotX);  // SkyrimSE.exe+733E8D
	}

private:
	static bool NewBeam(uint32_t* handle, RE::Projectile** proj)
	{
		auto found = _RefHandle__get(handle, proj);
		if (!found || !*proj)
			return found;

		return !is_CustomPosType(*proj);
	}

	static void update_node_pos(float x, float y, float z, RE::Projectile* proj)
	{
		if (auto node = proj->Get3D()) {
			if (!is_CustomPosType(proj)) {
				node->local.translate.x = x;
				node->local.translate.y = y;
				node->local.translate.z = z;
			}
		}
	}

	static void UpdatePos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_CustomPosType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static void UpdateRotZ(RE::Projectile* proj, float rot_Z)
	{
		if (!is_CustomPosType(proj)) {
			_j_TESObjectREFR__SetRotationZ_14074FEE0(proj, rot_Z);
		}
	}

	static void UpdateRotX(RE::Projectile* proj, float rot_X)
	{
		if (!is_CustomPosType(proj)) {
			_Projectile__SetRotationX(proj, rot_X);
		}
	}

	static inline REL::Relocation<decltype(NewBeam)> _RefHandle__get;
	static inline REL::Relocation<decltype(UpdatePos)> _TESObjectREFR__SetPosition_140296910;
	static inline REL::Relocation<decltype(UpdateRotZ)> _j_TESObjectREFR__SetRotationZ_14074FEE0;
	static inline REL::Relocation<decltype(UpdateRotX)> _Projectile__SetRotationX;
};

class NormLightingsHook
{
public:
	static void Hook()
	{
		_BeamProjectile__ctor =
			SKSE::GetTrampoline().write_call<5>(REL::ID(42928).address() + 0x185, Ctor);  // SkyrimSE.exe+74B2F5
	}

private:
	static RE::BeamProjectile* Ctor(RE::BeamProjectile* proj, void* LaunchData) {
		proj = _BeamProjectile__ctor(proj, LaunchData);
		if (auto spell = proj->spell; spell && spell->GetCastingType() == RE::MagicSystem::CastingType::kFireAndForget) {
			set_CustomPosType(proj);
		}
		return proj;
	}

	static inline REL::Relocation<decltype(Ctor)> _BeamProjectile__ctor;
};

class AutoAimHook
{
public:
	static void Hook()
	{
		_Projectile__apply_gravity =
			SKSE::GetTrampoline().write_call<5>(REL::ID(43006).address() + 0x69, change_direction);  // SkyrimSE.exe+751309
	}

private:
	static bool change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime)
	{
		bool ans = _Projectile__apply_gravity(proj, dV, dtime);
		if (is_AutoAimType(proj)) {
			AutoAim::change_direction(proj, dV, dtime);
		}
		return ans;
	}

	static inline REL::Relocation<decltype(change_direction)> _Projectile__apply_gravity;
};

class CoolFireballHook
{
public:
	static void Hook()
	{
		_MissileProjectile__ctor =
			SKSE::GetTrampoline().write_call<5>(REL::ID(42928).address() + 0x219, Ctor);  // SkyrimSE.exe+74B389
	}

private:
	static RE::MissileProjectile* Ctor(RE::MissileProjectile* proj, void* LaunchData)
	{
		proj = _MissileProjectile__ctor(proj, LaunchData);
		if (auto type = is_homie(proj); type.first != AutoAimTypes::Normal) {
			set_AutoAimType(proj, type);
		}
		return proj;
	}

	static inline REL::Relocation<decltype(Ctor)> _MissileProjectile__ctor;
};

class PlayerCharacterHook
{
public:
	static void Hook() { _Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0])).write_vfunc(0xad, Update); }

private:
	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);
		DebugAPI_IMPL::DebugAPI::Update();
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};
