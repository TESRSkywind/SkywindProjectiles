#pragma once
#define NOMINMAX
#include "SimpleIni.h"

class Settings
{
	static constexpr auto ini_path = "Data/skse/plugins/FenixProjectilesAPI.ini"sv;

	static bool ReadBool(const CSimpleIniA& ini, const char* section, const char* setting, bool& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = ini.GetBoolValue(section, setting);
			return true;
		}
		return false;
	}

	static bool ReadFloat(const CSimpleIniA& ini, const char* section, const char* setting, float& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = static_cast<float>(ini.GetDoubleValue(section, setting));
			return true;
		}
		return false;
	}

	static bool ReadUint32(const CSimpleIniA& ini, const char* section, const char* setting, uint32_t& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = static_cast<uint32_t>(ini.GetLongValue(section, setting));
			return true;
		}
		return false;
	}

public:
	static inline bool NormLightingsEnabled = true;

	static inline bool Enable = true;
	static inline bool CursorCircle = true;
	static inline bool CursorDetected = true;

	static void ReadSettings()
	{
		CSimpleIniA ini;
		ini.LoadFile(ini_path.data());

		ReadBool(ini, "General", "bNormLightningsEnabled", NormLightingsEnabled);

		ReadBool(ini, "Debug", "bEnable", Enable);
		ReadBool(ini, "Debug", "bCursorCircle", CursorCircle);
		ReadBool(ini, "Debug", "bCursorDetected", CursorDetected);
	}
};
