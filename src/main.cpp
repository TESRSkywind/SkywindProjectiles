extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef DEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef DEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

#include "Hooks.h"
#include "Settings.h"
#include "AutoAim.h"
#include "Multicast.h"
#include "Emittors.h"

class InputHandler : public RE::BSTEventSink<RE::InputEvent*>
{
public:
	static InputHandler* GetSingleton()
	{
		static InputHandler singleton;
		return std::addressof(singleton);
	}

	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* e,
		RE::BSTEventSource<RE::InputEvent*>*) override
	{
		if (!*e)
			return RE::BSEventNotifyControl::kContinue;

		if (auto buttonEvent = (*e)->AsButtonEvent();
			buttonEvent && buttonEvent->HasIDCode() &&
			(buttonEvent->IsDown() || buttonEvent->IsPressed())) {
			if (int key = buttonEvent->GetIDCode(); key == 71) {
				reset_json();
				Settings::ReadSettings();
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	void enable() {
		if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
			input->AddEventSink(this);
		}
	}
};

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Settings::ReadSettings();
		read_json();

		InputHandler::GetSingleton()->enable();

		PaddingsProjectileHook::Hook();
		InitStartPosHook::Hook();
		MultipleBeamsHook::Hook();

		if (Settings::NormLightingsEnabled)
			NormLightingsHook::Hook();

		SetNewTypeHook::Hook();

		AutoAim::install();
		ManyProjs::install();
		Emitters::install();

		if (Settings::Enable) {
			DebugAPIHook::Hook();
		}

		break;
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
	if (!g_messaging) {
		logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
		return false;
	}

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 10);

	g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

	logger::info("loaded");
	return true;
}
