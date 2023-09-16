#include "FastEmitter.h"
#include "RuntimeData.h"

namespace FastEmitters
{
	namespace Data
	{
		auto get_FastEmitterType(RE::Projectile* proj) { return get_runtime_data(proj).get_FastEmitterType(); }
		void set_FastEmitterType(RE::Projectile* proj, Types type) { get_runtime_data(proj).set_FastEmitterType(type); }
		bool is_FastEmitter(RE::Projectile* proj) { return get_FastEmitterType(proj) != FastEmitters::Types::None; }
		void set_normalType(RE::Projectile* proj) { get_runtime_data(proj).set_FastEmitterType(FastEmitters::Types::None); }
	}

	void call_func(RE::Projectile* proj, float dtime, Types type)
	{
		switch (type) {
		case FastEmitters::Types::AccelerateToMaxSpeed:
			{
				constexpr float SEC_TO_MAXSPEED = 3.0f;

				// y = a*e^bx, a = M/X, b = ln X / N
				//constexpr float X = 70.0f;
				constexpr float LN_X = 4.248495242049359f;  // ln 70
				float b = LN_X / SEC_TO_MAXSPEED;
				float cur_speed = proj->linearVelocity.Length();
				float add_to_speed = exp(b * dtime);
				float new_speed = cur_speed * add_to_speed;
				proj->linearVelocity *= (new_speed / cur_speed);
			}
			break;
		case FastEmitters::Types::Unused2:
		case FastEmitters::Types::Unused3:
		case FastEmitters::Types::None:
		case FastEmitters::Types::Total:
		default:
			break;
		}
	}

	void onUpdate(RE::Projectile* proj, float dtime) { call_func(proj, dtime, Data::get_FastEmitterType(proj)); }

	void onCreated(RE::Projectile* proj, Types type) { Data::set_FastEmitterType(proj, type); }

	namespace Hooks
	{
		class FastEmitterHook
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
				if (Data::is_FastEmitter(proj)) {
					onUpdate(proj, dtime);
				}

				return _smth_explode(proj, dtime);
			}

			static void* AddImpact(RE::Projectile* proj, RE::TESObjectREFR* a2, RE::NiPoint3* a3, RE::NiPoint3* a_velocity,
				RE::hkpCollidable* a_collidable, uint32_t a6, char a7)
			{
				auto ans = _AddImpact(proj, a2, a3, a_velocity, a_collidable, a6, a7);
				if (Data::is_FastEmitter(proj)) {
					Data::set_normalType(proj);
				}
				return ans;
			}

			static void sub_140BEDB60(char* sound)
			{
				_sub_140BEDB60(sound);
				auto proj = reinterpret_cast<RE::Projectile*>(sound - 0x128);
				if (Data::is_FastEmitter(proj)) {
					Data::set_normalType(proj);
				}
			}

			static inline REL::Relocation<decltype(smth_explode)> _smth_explode;
			static inline REL::Relocation<decltype(AddImpact)> _AddImpact;
			static inline REL::Relocation<decltype(sub_140BEDB60)> _sub_140BEDB60;
		};
	}

	void install() { Hooks::FastEmitterHook::Hook(); }
}
