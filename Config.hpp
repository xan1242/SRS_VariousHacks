#pragma once
#include "framework.h"
#include <filesystem>

#ifndef CONFIG_HPP
#define CONFIG_HPP

struct Config
{
	struct BootFlow
	{
		bool SkipIntroVideo = true;
		bool SkipLegalScreens = true;
	} bootflow;

	struct Graphics
	{
		struct Window
		{
			uint32_t Width = 0;
			uint32_t Height = 0;
			WindowedModes WindowMode = WNDMODE_DEFAULT;
			bool bAlwaysOnTop = false;
		} window;

		struct AspectRatio
		{
			bool FixHUD = true;
			bool FixFOV = true;
		} aspectratio;

		struct Effects
		{
			bool RoadCarReflections = false;
			bool HighestLods = false;
			bool InfiniteNosFlame = false;

			struct NosFlame
			{
				int ColorRed = 0;
				int ColorGreen = 0;
				int ColorBlue = 0;
				int ColorAlpha = 0;
			} nosflame;
		} effects;

	} graphics;

	struct Gameplay
	{
		bool MoreCameraModes = false;
		bool CopCarInDealer = false;
		int MaxRespect = 250;
		bool NoDecalRestrictions = false;
		bool ShowHiddenVinyl = false;
		bool NoEngineRestrictions = false;
		int NumPaintColors = 21;

		struct Assists
		{
			bool NoABS = false;

			struct Steering
			{
				float YawSteerAssist = -1.0f;
				float AssistWeight = -1.0f;
				float ReturnSpring = -1.0f;
				SteeringAssistType AssistType = STEERINGASSIST_UNK;
			} steering;
		} assists;
	} gameplay;

	struct HotKeys
	{
		int vkToggleHood = 0;
		int vkToggleDrawHUD = 0;
	} hotkeys;

	struct Misc
	{
		bool DisableMinimizeOnAltTab = false;
		bool DisableDInputSetCooperativeLevel_Keyboard = true;
		bool DisableDInputSetCooperativeLevel_Mouse = false;
		bool FixAltF4 = true;
		bool Console = false;
		bool NoMouse = false;
	} misc;

	static Config& Get() 
	{
		static Config sConfig;
		return sConfig;
	};

	static Config& ReadConfig(const std::filesystem::path& iniPath);
};

#endif