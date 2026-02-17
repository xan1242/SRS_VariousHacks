#include "framework.h"
#include "includes/injector/injector.hpp"

#define FINI_CRLF_NEWLINE
#define FINI_WRITEINTBOOLS
#define FINI_SEMICOLONSONLY
#define FINI_NOINLINECOMMENTSPARSE
#define FINI_NOPRECEDINGCOMMENTSPARSE
#define FINI_USEHASH
#define FINI_READONLY
#include "includes/fini/fini.hpp"
#include "includes/ModPath.hpp"
#include <filesystem>

int hk_ToggleHood;
int hk_ToggleDrawHUD;
bool ToggleHood = false;
bool CopCarInDealer = false;
bool ShowHiddenVinyl = false;

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

void MainLoop()
{
	__asm pushad;

	ProcessEngineAnimation(Game::RaceCars[0]);
	ProcessEngineAnimation(Game::GarageCar[0]);

	if (GetAsyncKeyState(hk_ToggleHood) & 1)
	{
		ToggleHood = !ToggleHood;
	}

	if (GetAsyncKeyState(hk_ToggleDrawHUD) & 1)
	{
		*Game::DrawHUD = !*Game::DrawHUD;
	}

	if (CopCarInDealer)
	{
		*Game::CopCarUnlocked = true;
		*Game::CopCarVisible = true;
	}

	if (ShowHiddenVinyl)
	{
		Game::SetShowHiddenVinyl();
	}

	__asm popad;
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


void Init()
{
	fINI::Reader ini;
	int iniReadError = ini.Open(ModPath::GetThisModulePath<std::filesystem::path>().replace_extension("ini"));
	if (iniReadError)
		return;

	bool RoadCarReflections = false;
	bool HighestLods = false;
	bool InfiniteNosFlame = false;
	bool MoreCameraModes = false;
	bool NoDecalRestrictions = false;
	bool NoEngineRestrictions = false;
	bool Console = false;
	int MaxRespect = 250;
	int NumPaintColors = 21;

	int nosColorRed = 0;
	int nosColorGreen = 0;
	int nosColorBlue = 0;
	int nosColorAlpha = 0;



	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("RoadCarReflections"), RoadCarReflections);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("CopCarInDealer"), CopCarInDealer);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("HighestLods"), HighestLods);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("InfiniteNosFlame"), InfiniteNosFlame);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("MoreCameraModes"), MoreCameraModes);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("NoDecalRestrictions"), NoDecalRestrictions);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("NoEngineRestrictions"), NoEngineRestrictions);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("Console"), Console);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("ShowHiddenVinyl"), ShowHiddenVinyl);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("MaxRespect"), MaxRespect);
	ini.ReadValue(FINI_HASH("GENERAL"), FINI_HASH("NumPaintColors"), NumPaintColors);

	ini.ReadValue(FINI_HASH("NOS_FLAME"), FINI_HASH("Red"), nosColorRed);
	ini.ReadValue(FINI_HASH("NOS_FLAME"), FINI_HASH("Green"), nosColorGreen);
	ini.ReadValue(FINI_HASH("NOS_FLAME"), FINI_HASH("Blue"), nosColorBlue);
	ini.ReadValue(FINI_HASH("NOS_FLAME"), FINI_HASH("Alpha"), nosColorAlpha);


	ini.ReadValue(FINI_HASH("HOT_KEYS"), FINI_HASH("ToggleHood"), hk_ToggleHood);
	ini.ReadValue(FINI_HASH("HOT_KEYS"), FINI_HASH("ToggleDrawHUD"), hk_ToggleDrawHUD);


	if (RoadCarReflections)
	{
		injector::WriteMemory<uint8_t>(0x006834A7, 0, true);
	}


	if (HighestLods)
	{
		injector::MakeNOP(0x00551B12, 2, true);
	}

	if (InfiniteNosFlame)
	{
		injector::MakeNOP(0x006A86C4, 2, true);
	}

	if (MoreCameraModes)
	{
		injector::MakeJMP(0x00523659, InitCameraModesCave, true);
	}

	if (NoDecalRestrictions)
	{
		injector::WriteMemory<uint8_t>(0x0055E256, 0xEB, true);
	}

	if (NoEngineRestrictions)
	{
		injector::WriteMemory<uint16_t>(0x0050ED7B, 0x01B0, true);
	}

	if (Console)
	{
		injector::MakeJMP(0x006227A0, printf, true);

		AllocConsole();
		AttachConsole(ATTACH_PARENT_PROCESS);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}


	injector::WriteMemory(Game::MaxRespect1, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect2, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect3, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect4, MaxRespect, true);

	injector::WriteMemory(0x005CDA26, NumPaintColors * 20, true);

	injector::WriteMemory(0x006A6CD4, nosColorRed, true);
	injector::WriteMemory(0x006A6CCF, nosColorGreen, true);
	injector::WriteMemory(0x006A6CC3, nosColorBlue, true);
	injector::WriteMemory(0x006A6CBE, nosColorAlpha, true);

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