#include "Following.h"

namespace Capturing
{
	namespace Hooks
	{
		class CapturingHook
		{
		public:
			static void Hook()
			{
				_Projectile__apply_gravity = SKSE::GetTrampoline().write_call<5>(REL::ID(43006).address() + 0x69,
					change_direction);  // SkyrimSE.exe+751309
			}

		private:
			static void change_shooter(RE::Projectile* proj, RE::Actor* shooter)
			{
				RE::bhkRefObject* ref_ptr = (RE::bhkRefObject*)proj->unk0E0;
				RE::hkpWorldObject* world_ptr = (RE::hkpWorldObject*)ref_ptr->referencedObject.get();
				auto collidable = world_ptr->GetCollidableRW();

				uint32_t filter;
				shooter->GetCollisionFilterInfo(filter);
				collidable->broadPhaseHandle.collisionFilterInfo &= (0x0000FFFF);
				collidable->broadPhaseHandle.collisionFilterInfo |= (filter << 16);

				proj->shooter = shooter->GetHandle();
				proj->SetActorCause(shooter->GetActorCause());
			}

			static bool change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime)
			{
				if (auto _shooter = proj->shooter.get().get(); _shooter->As<RE::Actor>() && !_shooter->IsPlayerRef()) {
					//auto shooter = _shooter->As<RE::Actor>();
					auto player = RE::PlayerCharacter::GetSingleton();

					float len2 = (player->GetPosition() - proj->GetPosition()).SqrLength();
					if (len2 <= 20000.0f && player->IsCasting(RE::TESForm::LookupByID<RE::SpellItem>(0x12FCC))) {
						change_shooter(proj, player);
						Following::onCreated(proj, FollowTypes::Hand, 0, UINT32_MAX);
					}
				}

				return _Projectile__apply_gravity(proj, dV, dtime);
			}

			static inline REL::Relocation<decltype(change_direction)> _Projectile__apply_gravity;
		};

	}
}
