#include "RuntimeData.h"

FenixProjsRuntimeData& get_runtime_data(RE::Projectile* proj)
{
	return (FenixProjsRuntimeData&)(uint32_t&)proj->pad164;
}

bool is_CustomPosType(RE::Projectile* proj)
{
	return get_runtime_data(proj).isCustomPos();
}
void set_CustomPosType(RE::Projectile* proj) { get_runtime_data(proj).setCustomPos(); }
void init_NormalType(RE::Projectile* proj) { get_runtime_data(proj).set_NormalType(); }
