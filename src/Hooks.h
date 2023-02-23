#pragma once
#include "FenixProjectilesAPI.h"
#include "NewProjectiles.h"
#include "RuntimeData.h"
#include "Emittors.h"

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
		init_NormalType(proj);
	}

	static void __fastcall LoadGame(RE::Projectile* proj, RE::BGSLoadGameBuffer* buf)
	{
		_TESObjectREFR__ReadFromSaveGame_140286FD0(proj, buf);
		init_NormalType(proj);
	}

	static inline REL::Relocation<decltype(Ctor)> _TESForm__SetInitedFormFlag_140194B90;
	static inline REL::Relocation<decltype(LoadGame)> _TESObjectREFR__ReadFromSaveGame_140286FD0;
};
/*
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
*/
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

			FenixUtils::add_trampoline<5, 42586, 0x2c1>(&xbyakCode);  // SkyrimSE.exe+733F81
		}

		_TESObjectREFR__SetPosition_140296910 =
			trmpl.write_call<5>(REL::ID(42586).address() + 0x2db, UpdatePos);  // SkyrimSE.exe+733F9B
		
		_Projectile__SetRotation = trmpl.write_call<5>(REL::ID(42586).address() + 0x249, UpdateRot);  // SkyrimSE.exe+733F09
	}

private:
	static bool NewBeam(uint32_t* handle, RE::Projectile** proj)
	{
		auto found = _RefHandle__get(handle, proj);
		if (!found || !*proj)
			return found;

		return !is_MyBeamType(*proj);
	}

	static void update_node_pos(float x, float y, float z, RE::Projectile* proj)
	{
		if (auto node = proj->Get3D()) {
			if (!is_MyBeamType(proj)) {
				node->local.translate.x = x;
				node->local.translate.y = y;
				node->local.translate.z = z;
			}
		}
	}

	static void UpdatePos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_MyBeamType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static void UpdateRot(RE::Projectile* proj, float rot_X)
	{
		if (!is_MyBeamType(proj)) {
			_Projectile__SetRotation(proj, rot_X);
		}
	}

	static inline REL::Relocation<decltype(NewBeam)> _RefHandle__get;
	static inline REL::Relocation<decltype(UpdatePos)> _TESObjectREFR__SetPosition_140296910;
	static inline REL::Relocation<decltype(UpdateRot)> _Projectile__SetRotation;
};
/*
class MultipleFlamesHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		{
			// SkyrimSE.exe+73d9d0
			uintptr_t ret_addr = REL::ID(42727).address() + 0x241;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func_addr, uintptr_t ret_addr)
				{
					Xbyak::Label nocancel;

					// rsi  = proj
					// xmm7 -- xmm9 = node pos
					movss(xmm0, xmm7);
					movss(xmm1, xmm8);
					movss(xmm2, xmm9);
					mov(r9, rsi);
					mov(rax, func_addr);
					call(rax);
					mov(rax, ret_addr);
					jmp(rax);
				}
			} xbyakCode{ uintptr_t(update_node_pos), ret_addr };

			FenixUtils::add_trampoline<5, 42727, 0x22f>(
				&xbyakCode);  // SkyrimSE.exe+73d9d0
		}

		_j_TESObjectREFR__SetRotationZ_14074FEE0 = trmpl.write_call<5>(
			REL::ID(42727).address() + 0x133, UpdateRotZ);  // SkyrimSE.exe+73d9d0
		_TESObjectREFR__SetPosition_140296910 = trmpl.write_call<5>(
			REL::ID(42727).address() + 0x24b, UpdatePos);  // SkyrimSE.exe+73d9d0
		_Projectile__SetRotationX = trmpl.write_call<5>(REL::ID(42727).address() + 0x146,
			UpdateRotX);  // SkyrimSE.exe+73d9d0
	}

private:
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

	static void UpdatePos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_CustomPosType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static inline REL::Relocation<decltype(UpdateRotZ)>
		_j_TESObjectREFR__SetRotationZ_14074FEE0;
	static inline REL::Relocation<decltype(UpdateRotX)> _Projectile__SetRotationX;
	static inline REL::Relocation<decltype(UpdatePos)>
		_TESObjectREFR__SetPosition_140296910;
};
*/
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
			proj->flags.set(RE::Projectile::Flags::kUseOrigin);
			proj->flags.reset(RE::Projectile::Flags::kAutoAim);
		}
		return proj;
	}

	static inline REL::Relocation<decltype(Ctor)> _BeamProjectile__ctor;
};

class SetNewTypeHook
{
public:
	static void Hook()
	{
		_Usage1 = SKSE::GetTrampoline().write_call<5>(REL::ID(13629).address() + 0x130,
			CreateProjectile_14074B170_1);  // SkyrimSE.exe+16CDD0 -- placeatme
		_Usage2 = SKSE::GetTrampoline().write_call<5>(REL::ID(17693).address() + 0xe82,
			CreateProjectile_14074B170_2);  // SkyrimSE.exe+2360C2 -- TESObjectWEAP::Fire_140235240
		_Usage3 = SKSE::GetTrampoline().write_call<5>(REL::ID(33672).address() + 0x377,
			CreateProjectile_14074B170_3);  // SkyrimSE.exe+550A37 -- ActorMagicCaster::castProjectile
		_Usage4 = SKSE::GetTrampoline().write_call<5>(REL::ID(35450).address() + 0x20e,
			CreateProjectile_14074B170_4);  // SkyrimSE.exe+5A897E -- ChainExplosion
	}

private:
	static void set_custom_type(uint32_t handle) {
		RE::TESObjectREFRPtr _refr;
		RE::LookupReferenceByHandle(handle, _refr);
		if (auto proj = _refr.get() ? _refr.get()->As<RE::Projectile>() : nullptr) {
			AutoAim::onCreated(proj);
			Emitters::onCreated(proj);
		}
	}

	static uint32_t* CreateProjectile_14074B170_1(uint32_t* handle, void* ldata)
	{
		handle = _Usage1(handle, ldata);
		set_custom_type(*handle);
		return handle;
	}
	static uint32_t* CreateProjectile_14074B170_2(uint32_t* handle, void* ldata)
	{
		handle = _Usage2(handle, ldata);
		set_custom_type(*handle);
		return handle;
	}
	static uint32_t* CreateProjectile_14074B170_3(uint32_t* handle, void* ldata)
	{
		handle = _Usage3(handle, ldata);
		set_custom_type(*handle);
		return handle;
	}
	static uint32_t* CreateProjectile_14074B170_4(uint32_t* handle, void* ldata)
	{
		handle = _Usage4(handle, ldata);
		set_custom_type(*handle);
		return handle;
	}

	using func_type = decltype(CreateProjectile_14074B170_1);

	static inline REL::Relocation<func_type> _Usage1;
	static inline REL::Relocation<func_type> _Usage2;
	static inline REL::Relocation<func_type> _Usage3;
	static inline REL::Relocation<func_type> _Usage4;
};

class DebugAPIHook
{
public:
	static void Hook() { _Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0])).write_vfunc(0xad, Update); }

private:
	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);

		SKSE::GetTaskInterface()->AddUITask([]() { DebugAPI_IMPL::DebugAPI::Update(); });
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};
