#include "framework.h"
#include "includes/injector/injector.hpp"

#include "Config.hpp"
#include "WidescreenFix.hpp"
#include "WindowedMode.hpp"
#include "MainLoop.hpp"
#include "GameExit.hpp"
#include "Assists.hpp"

#include "includes/ModPath.hpp"
#include <filesystem>

bool ToggleHood = false;

bool bAltF4OldState = false;

namespace Game
{
	auto RaceCars = (uint32_t**)0x007708F4;
	auto GarageCar = (uint32_t**)0x00791BC4;

	bool* AllowedCameraModes = (bool*)0x007D5C74;

	bool* CopCarVisible = (bool*)0x007A557D;
	bool* CopCarUnlocked = (bool*)0x007A5580;

	int* MaxRespect1 = (int*)0x0053E917;
	int* MaxRespect2 = (int*)0x0053E929;
	int* MaxRespect3 = (int*)0x0053D1E8;
	int* MaxRespect4 = (int*)0x0053D1EF;

	bool* DrawHUD = (bool*)0x007D5CA0;

	auto SetShowHiddenVinyl = (void(__stdcall*)())0x005198B0;
	auto ProcessEngineAnimation = (void(__thiscall*)(void*, int))0x00541DF0;
	auto SetEngineAnimationState = (void(__thiscall*)(void*, int))0x00541D20;
	auto IsEngineAnimationState = (bool(__thiscall*)(void*, int))0x0053F5D0;
}

void ProcessEngineAnimation(uint32_t* playerCar)
{
	if (playerCar && *playerCar == 0x0073B008)
	{
		Game::ProcessEngineAnimation(playerCar, 0);
		if (!ToggleHood)
		{
			if (Game::IsEngineAnimationState(playerCar, 0))
			{
				Game::SetEngineAnimationState(playerCar, 2);
			}
		}
		else if (!Game::IsEngineAnimationState(playerCar, 0))
		{
			Game::SetEngineAnimationState(playerCar, 2);
		}
	}
}

bool IsAltF4Down()
{
	return (((GetAsyncKeyState(VK_MENU) & 0x8000) ||
		(GetAsyncKeyState(VK_LMENU) & 0x8000)) &&
		(GetAsyncKeyState(VK_F4) & 0x8000));
}

void OnMainLoop(void*, void*)
{
	Config& cfg = Config::Get();
	ProcessEngineAnimation(Game::RaceCars[0]);
	ProcessEngineAnimation(Game::GarageCar[0]);

	if (GetAsyncKeyState(cfg.hotkeys.vkToggleHood) & 1)
	{
		ToggleHood = !ToggleHood;
	}

	if (GetAsyncKeyState(cfg.hotkeys.vkToggleDrawHUD) & 1)
	{
		*Game::DrawHUD = !*Game::DrawHUD;
	}

	if (cfg.gameplay.CopCarInDealer)
	{
		*Game::CopCarUnlocked = true;
		*Game::CopCarVisible = true;
	}

	if (cfg.gameplay.ShowHiddenVinyl)
	{
		Game::SetShowHiddenVinyl();
	}

	// hacky workaround for now -- game uses MFC and some weird shenanigans...
	if (cfg.misc.FixAltF4)
	{
		bool bAltF4State = IsAltF4Down();

		if (bAltF4State != bAltF4OldState)
		{
			if (!bAltF4State) // negative edge
			{
				GameExit::Exit();
			}
		}

		bAltF4OldState = bAltF4State;
	}
}


void InitCameraModes()
{
	Game::AllowedCameraModes[1] = true;
	Game::AllowedCameraModes[2] = true;
	Game::AllowedCameraModes[3] = true;
	Game::AllowedCameraModes[18] = true;
	Game::AllowedCameraModes[25] = true;
	Game::AllowedCameraModes[30] = true;
}

void __declspec(naked) InitCameraModesCave()
{
	__asm
	{
		pushad;
		call InitCameraModes;
		popad;
		retn;
	}
}


#pragma runtime_checks( "", off )

// resolves game not closing on window close
uintptr_t p_CWnd_Destructor;
void __stdcall CWnd_Destructor_Hook(uint32_t deleteFlag)
{
	uintptr_t that;
	_asm mov that, ecx

	reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(p_CWnd_Destructor)(that, deleteFlag);

	//PostQuitMessage(0);
	GameExit::Exit();
}

#pragma runtime_checks( "", restore )


void Init()
{
	Config& cfg = Config::ReadConfig(ModPath::GetThisModulePath<std::filesystem::path>().replace_extension("ini"));
	
	WidescreenFix::SetRes(cfg.graphics.window.Width, cfg.graphics.window.Height);
	WidescreenFix::SetFixFOV(cfg.graphics.aspectratio.FixFOV);
	WidescreenFix::SetFixHUD(cfg.graphics.aspectratio.FixHUD);
	WidescreenFix::Init();

	WindowedMode::Init(cfg.graphics.window.WindowMode);

	MainLoop::Init();
	GameExit::Init();

	// patch window destructor to fix game closing on window close
	constexpr size_t vtidx_CWnd_Destructor = 1;
	uintptr_t loc_403B28 = 0x403B28;
	uintptr_t* CWnd_vtbl = *reinterpret_cast<uintptr_t**>(loc_403B28 + 2);
	p_CWnd_Destructor = CWnd_vtbl[vtidx_CWnd_Destructor];
	injector::WriteMemory(&CWnd_vtbl[vtidx_CWnd_Destructor], &CWnd_Destructor_Hook, true);
	

	if (cfg.graphics.effects.RoadCarReflections)
	{
		injector::WriteMemory<uint8_t>(0x006834A7, 0, true);
	}


	if (cfg.graphics.effects.HighestLods)
	{
		injector::MakeNOP(0x00551B12, 2, true);
	}

	if (cfg.graphics.effects.InfiniteNosFlame)
	{
		injector::MakeNOP(0x006A86C4, 2, true);
	}

	if (cfg.gameplay.MoreCameraModes)
	{
		injector::MakeJMP(0x00523659, InitCameraModesCave, true);
	}

	if (cfg.gameplay.NoDecalRestrictions)
	{
		injector::WriteMemory<uint8_t>(0x0055E256, 0xEB, true);
	}

	if (cfg.gameplay.NoEngineRestrictions)
	{
		injector::WriteMemory<uint16_t>(0x0050ED7B, 0x01B0, true);
	}

	if (cfg.misc.Console)
	{
		injector::MakeJMP(0x006227A0, printf, true);

		AllocConsole();
		AttachConsole(ATTACH_PARENT_PROCESS);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}

	if (cfg.misc.DisableMinimizeOnAltTab)
	{
		injector::MakeRET(0x403B70, 8);
		// yeet the print spam
		injector::MakeNOP(0x61DDA7, 5);
		injector::MakeNOP(0x61DDDC, 5);
		injector::MakeNOP(0x61DEEB, 5);
	}

	if (cfg.misc.DisableDInputSetCooperativeLevel_Keyboard)
	{
		injector::MakeJMP(0x61DB5A, 0x61DB7D);
	}

	if (cfg.misc.DisableDInputSetCooperativeLevel_Mouse)
	{
		injector::MakeJMP(0x61DC18, 0x61DC3B);
	}

	if (cfg.bootflow.SkipIntroVideo)
	{
		injector::MakeJMP(0x5B665E, 0x5B6709);
	}

	if (cfg.bootflow.SkipLegalScreens)
	{
		injector::MakeNOP(0x55A30A, 5);
	}

	if (cfg.misc.NoMouse)
	{
		// skip over all mouse input checks...
		injector::MakeJMP(0x61D078, 0x61D546);
	}


	injector::WriteMemory(Game::MaxRespect1, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect2, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect3, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect4, cfg.gameplay.MaxRespect, true);

	injector::WriteMemory(0x005CDA26, cfg.gameplay.NumPaintColors * 20, true);

	injector::WriteMemory(0x006A6CD4, cfg.graphics.effects.nosflame.ColorRed, true);
	injector::WriteMemory(0x006A6CCF, cfg.graphics.effects.nosflame.ColorGreen, true);
	injector::WriteMemory(0x006A6CC3, cfg.graphics.effects.nosflame.ColorBlue, true);
	injector::WriteMemory(0x006A6CBE, cfg.graphics.effects.nosflame.ColorAlpha, true);

	Assists::Init();
	Assists::SetABS(!cfg.gameplay.assists.NoABS);
	Assists::Steering::SetYawSteerAssist(cfg.gameplay.assists.steering.YawSteerAssist);
	Assists::Steering::SetAssistWeight(cfg.gameplay.assists.steering.AssistWeight);
	Assists::Steering::SetAssistType(cfg.gameplay.assists.steering.AssistType);

	MainLoop::Walker::AddToLoop(OnMainLoop);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}

	return TRUE;
}