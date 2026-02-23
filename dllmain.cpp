#include "framework.h"
#include "includes/injector/injector.hpp"

#include "Config.hpp"
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

void MainLoop()
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
				PostQuitMessage(0);
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

	PostQuitMessage(0);
}


float YawSteerAssist = -1.0f;
uintptr_t p_ParseFloatToken_YawSteerAssist;
float __stdcall ParseFloatToken_YawSteerAssist()
{
	uintptr_t that;
	_asm mov that, ecx

	float retVal = reinterpret_cast<float(__thiscall*)(uintptr_t)>(p_ParseFloatToken_YawSteerAssist)(that);

	if (YawSteerAssist < 0.0f)
		return retVal;

	return YawSteerAssist;
}

float AssistWeight = -1.0f;
uintptr_t p_ParseFloatToken_AssistWeight;
float __stdcall ParseFloatToken_AssistWeight()
{
	uintptr_t that;
	_asm mov that, ecx

	float retVal = reinterpret_cast<float(__thiscall*)(uintptr_t)>(p_ParseFloatToken_AssistWeight)(that);

	if (AssistWeight < 0.0f)
		return retVal;

	return AssistWeight;
}

SteeringAssistType AssistType = STEERINGASSIST_UNK;
uintptr_t p_ParseAssistType;
float __stdcall ParseAssistType_Hook()
{
	uintptr_t that;
	_asm mov that, ecx

	reinterpret_cast<void(__thiscall*)(uintptr_t)>(p_ParseAssistType)(that);

	if (AssistType >= STEERINGASSIST_DEFAULT)
		*reinterpret_cast<SteeringAssistType*>(that + 0x74) = AssistType;
}

#pragma runtime_checks( "", restore )


void Init()
{
	Config& cfg = Config::ReadConfig(ModPath::GetThisModulePath<std::filesystem::path>().replace_extension("ini"));
	
	// patch window destructor to fix game closing on window close
	constexpr size_t vtidx_CWnd_Destructor = 1;
	uintptr_t loc_403B28 = 0x403B28;
	uintptr_t* CWnd_vtbl = *reinterpret_cast<uintptr_t**>(loc_403B28 + 2);
	p_CWnd_Destructor = CWnd_vtbl[vtidx_CWnd_Destructor];
	injector::WriteMemory(&CWnd_vtbl[vtidx_CWnd_Destructor], &CWnd_Destructor_Hook, true);
	

	if (cfg.graphics.RoadCarReflections)
	{
		injector::WriteMemory<uint8_t>(0x006834A7, 0, true);
	}


	if (cfg.graphics.HighestLods)
	{
		injector::MakeNOP(0x00551B12, 2, true);
	}

	if (cfg.graphics.InfiniteNosFlame)
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

	if (cfg.bootflow.SkipIntroVideo)
	{
		injector::MakeJMP(0x5B665E, 0x5B6709);
	}

	if (cfg.bootflow.SkipLegalScreens)
	{
		injector::MakeNOP(0x55A30A, 5);
	}

	if (cfg.gameplay.assists.NoABS)
	{
		injector::MakeRangedNOP(0x6D0500, 0x006D0508);
	}


	injector::WriteMemory(Game::MaxRespect1, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect2, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect3, cfg.gameplay.MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect4, cfg.gameplay.MaxRespect, true);

	injector::WriteMemory(0x005CDA26, cfg.gameplay.NumPaintColors * 20, true);

	injector::WriteMemory(0x006A6CD4, cfg.graphics.nosflame.ColorRed, true);
	injector::WriteMemory(0x006A6CCF, cfg.graphics.nosflame.ColorGreen, true);
	injector::WriteMemory(0x006A6CC3, cfg.graphics.nosflame.ColorBlue, true);
	injector::WriteMemory(0x006A6CBE, cfg.graphics.nosflame.ColorAlpha, true);

	uintptr_t loc_6E5B63 = 0x6E5B63;
	p_ParseFloatToken_YawSteerAssist = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5B63));
	injector::MakeCALL(loc_6E5B63, ParseFloatToken_YawSteerAssist);
	YawSteerAssist = cfg.gameplay.assists.steering.YawSteerAssist;

	uintptr_t loc_6E5B23 = 0x6E5B23;
	p_ParseFloatToken_AssistWeight = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5B23));
	injector::MakeCALL(loc_6E5B23, ParseFloatToken_AssistWeight);
	AssistWeight = cfg.gameplay.assists.steering.AssistWeight;

	uintptr_t loc_6E5A81 = 0x6E5A81;
	p_ParseAssistType = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5A81));
	injector::MakeCALL(loc_6E5A81, ParseAssistType_Hook);
	AssistType = cfg.gameplay.assists.steering.AssistType;

	injector::MakeCALL(0x004044B8, MainLoop, true);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}

	return TRUE;
}