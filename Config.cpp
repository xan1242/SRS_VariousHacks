#include "framework.h"
#include "Config.hpp"

#define FINI_CRLF_NEWLINE
#define FINI_WRITEINTBOOLS
#define FINI_SEMICOLONSONLY
#define FINI_NOINLINECOMMENTSPARSE
#define FINI_NOPRECEDINGCOMMENTSPARSE
#define FINI_USEHASH
#define FINI_READONLY
#include "includes/fini/fini.hpp"

Config& Config::ReadConfig(const std::filesystem::path& iniPath)
{
	fINI::Reader ini;
	int iniReadError = ini.Open(iniPath);
	Config& sConfig = Config::Get();

	if (iniReadError)
		return sConfig;

	ini.ReadValue(FINI_HASH("BootFlow"), FINI_HASH("SkipIntroVideo"),   sConfig.bootflow.SkipIntroVideo);
	ini.ReadValue(FINI_HASH("BootFlow"), FINI_HASH("SkipLegalScreens"), sConfig.bootflow.SkipLegalScreens);

	ini.ReadValue(FINI_HASH("Graphics.Window"), FINI_HASH("Width"), sConfig.graphics.window.Width);
	ini.ReadValue(FINI_HASH("Graphics.Window"), FINI_HASH("Height"), sConfig.graphics.window.Height);
	ini.ReadEnum(FINI_HASH("Graphics.Window"), FINI_HASH("WindowedMode"), sConfig.graphics.window.WindowMode, WindowedModes::WNDMODE_MIN, WindowedModes::WNDMODE_MAX);
	ini.ReadValue(FINI_HASH("Graphics.Window"), FINI_HASH("AlwaysOnTop"), sConfig.graphics.window.bAlwaysOnTop);

	ini.ReadValue(FINI_HASH("Graphics.AspectRatio"), FINI_HASH("FixHUD"), sConfig.graphics.aspectratio.FixHUD);
	ini.ReadValue(FINI_HASH("Graphics.AspectRatio"), FINI_HASH("FixFOV"), sConfig.graphics.aspectratio.FixFOV);

	ini.ReadValue(FINI_HASH("Graphics.Effects"), FINI_HASH("RoadCarReflections"), sConfig.graphics.effects.RoadCarReflections);
	ini.ReadValue(FINI_HASH("Graphics.Effects"), FINI_HASH("HighestLods"),        sConfig.graphics.effects.HighestLods);
	ini.ReadValue(FINI_HASH("Graphics.Effects"), FINI_HASH("InfiniteNosFlame"),   sConfig.graphics.effects.InfiniteNosFlame);

	ini.ReadValue(FINI_HASH("Graphics.Effects.NosFlame"), FINI_HASH("ColorRed"),   sConfig.graphics.effects.nosflame.ColorRed);
	ini.ReadValue(FINI_HASH("Graphics.Effects.NosFlame"), FINI_HASH("ColorGreen"), sConfig.graphics.effects.nosflame.ColorGreen);
	ini.ReadValue(FINI_HASH("Graphics.Effects.NosFlame"), FINI_HASH("ColorBlue"),  sConfig.graphics.effects.nosflame.ColorBlue);
	ini.ReadValue(FINI_HASH("Graphics.Effects.NosFlame"), FINI_HASH("ColorAlpha"), sConfig.graphics.effects.nosflame.ColorAlpha);

	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("MoreCameraModes"),      sConfig.gameplay.MoreCameraModes);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("CopCarInDealer"),       sConfig.gameplay.CopCarInDealer);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("MaxRespect"),           sConfig.gameplay.MaxRespect);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("NoDecalRestrictions"),  sConfig.gameplay.NoDecalRestrictions);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("ShowHiddenVinyl"),      sConfig.gameplay.ShowHiddenVinyl);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("NoEngineRestrictions"), sConfig.gameplay.NoEngineRestrictions);
	ini.ReadValue(FINI_HASH("Gameplay"), FINI_HASH("NumPaintColors"),       sConfig.gameplay.NumPaintColors);

	ini.ReadValue(FINI_HASH("Gameplay.Assists"), FINI_HASH("NoABS"), sConfig.gameplay.assists.NoABS);

	ini.ReadValue(FINI_HASH("Gameplay.Assists.Steering"), FINI_HASH("YawSteerAssist"), sConfig.gameplay.assists.steering.YawSteerAssist);
	ini.ReadValue(FINI_HASH("Gameplay.Assists.Steering"), FINI_HASH("AssistWeight"),   sConfig.gameplay.assists.steering.AssistWeight);
	ini.ReadValue(FINI_HASH("Gameplay.Assists.Steering"), FINI_HASH("ReturnSpring"),   sConfig.gameplay.assists.steering.ReturnSpring);
	ini.ReadEnum(FINI_HASH("Gameplay.Assists.Steering"), FINI_HASH("AssistType"),      sConfig.gameplay.assists.steering.AssistType, STEERINGASSIST_MIN, STEERINGASSIST_MAX);

	ini.ReadValue(FINI_HASH("HotKeys"), FINI_HASH("ToggleHood"),    sConfig.hotkeys.vkToggleHood);
	ini.ReadValue(FINI_HASH("HotKeys"), FINI_HASH("ToggleDrawHUD"), sConfig.hotkeys.vkToggleDrawHUD);

	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("DisableMinimizeOnAltTab"), sConfig.misc.DisableMinimizeOnAltTab);
	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("DisableDInputSetCooperativeLevel_Keyboard"), sConfig.misc.DisableDInputSetCooperativeLevel_Keyboard);
	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("DisableDInputSetCooperativeLevel_Mouse"), sConfig.misc.DisableDInputSetCooperativeLevel_Mouse);
	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("FixAltF4"),                sConfig.misc.FixAltF4);
	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("Console"),                 sConfig.misc.Console);
	ini.ReadValue(FINI_HASH("Misc"), FINI_HASH("NoMouse"),                 sConfig.misc.NoMouse);

	return sConfig;
}
