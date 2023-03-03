#include "RuntimeData.h"

FenixProjsRuntimeData& get_runtime_data(RE::Projectile* proj)
{
	return (FenixProjsRuntimeData&)(uint32_t&)proj->pad164;
}

void init_NormalType(RE::Projectile* proj) { get_runtime_data(proj).set_NormalType(); }
