#include "framework.h"
#include "WindowedMode.hpp"
#include "includes/injector/injector.hpp"

#include <tuple>
#include <d3d9.h>


namespace WindowedMode
{
	HWND GameHWND;

	// #TODO separate these for each hook...
	uintptr_t p_CreateD3DDevice;

	static HWND GetHWNDFromCWnd(uintptr_t cwnd)
	{
		return *(HWND*)(cwnd + 0x20);
	}

	static DWORD GetDesiredWindowStyle()
	{
		WndConfig& cfg = WndConfig::Get();

		DWORD dwStyle = 0;
		if (cfg.bBorderlessWindowed)
		{
			dwStyle = WS_POPUP | WS_VISIBLE;
		}
		else
		{
			dwStyle = WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE; // #TODO: read nCmdShow from WinMain!!!
			if (cfg.bEnableWindowResize)
				dwStyle |= WS_MAXIMIZEBOX | WS_THICKFRAME;
		}

		return dwStyle;
	}

	static DWORD GetDesiredWindowExStyle()
	{
		WndConfig& cfg = WndConfig::Get();
		DWORD dwExStyle = 0;
		if (cfg.bAlwaysOnTop)
			dwExStyle |= WS_EX_TOPMOST;

		return dwExStyle;
	}

	static void YeetEdge()
	{
		// silly game puts the edges on the window.... or rather - doesn't modify the MFC defaults
		SetWindowLong(GameHWND, GWL_EXSTYLE, GetDesiredWindowExStyle());
	}

	BOOL WINAPI CenterWindowPosition(HWND hWnd, int nWidth, int nHeight)
	{
		int newWidth = nWidth;
		int newHeight = nHeight;

		int DesktopW = GetSystemMetrics(SM_CXSCREEN);
		int DesktopH = GetSystemMetrics(SM_CYSCREEN);

		WndConfig& cfg = WndConfig::Get();

		if (cfg.bScaleWindow)
		{
			float fAspectRatio = static_cast<float>(nWidth) / static_cast<float>(nHeight);
			newHeight = DesktopH;
			newWidth = static_cast<int>(newHeight * fAspectRatio);
		}

		if (cfg.bStretchWindow)
		{
			newHeight = DesktopH;
			newWidth = DesktopW;
		}

		int xPos = (DesktopW - (newWidth)) / 2;
		int yPos = (DesktopH - (newHeight)) / 2;

		RECT windowRect = { xPos , yPos , xPos + newWidth , yPos + newHeight };

		if (!cfg.bBorderlessWindowed)
		{
			LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
			AdjustWindowRect(&windowRect, lStyle, FALSE);
		}

		return SetWindowPos(hWnd, 0, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	std::tuple<int, int, int, int> beforeCreateWindow(int nWidth, int nHeight)
	{
		WndConfig& cfg = WndConfig::Get();

		int newWidth = nWidth;
		int newHeight = nHeight;

		int DesktopW = GetSystemMetrics(SM_CXSCREEN);
		int DesktopH = GetSystemMetrics(SM_CYSCREEN);

		if (cfg.bScaleWindow)
		{
			float fAspectRatio = static_cast<float>(nWidth) / static_cast<float>(nHeight);
			newHeight = DesktopH;
			newWidth = static_cast<int>(newHeight * fAspectRatio);
		}

		if (cfg.bStretchWindow)
		{
			newHeight = DesktopH;
			newWidth = DesktopW;
		}

		int WindowPosX = (int)(((float)DesktopW / 2.0f) - ((float)newWidth / 2.0f));
		int WindowPosY = (int)(((float)DesktopH / 2.0f) - ((float)newHeight / 2.0f));

		return std::make_tuple(WindowPosX, WindowPosY, newWidth, newHeight);
	}

	void afterCreateWindow()
	{
		LONG lStyle = GetDesiredWindowStyle();
		WndConfig& cfg = WndConfig::Get();

		SetWindowLong(GameHWND, GWL_STYLE, lStyle);

		YeetEdge();

		RECT clientRect;
		if (cfg.bBorderlessWindowed)
		{
			if (GetWindowRect(GameHWND, &clientRect))
				CenterWindowPosition(GameHWND, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		}
		else if (GetClientRect(GameHWND, &clientRect))
			CenterWindowPosition(GameHWND, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}

#pragma runtime_checks( "", off )

	static bool __stdcall CreateD3DDevice_Hook(D3DPRESENT_PARAMETERS* d3dpp, uint32_t creationFlags)
	{
		uintptr_t that;
		_asm mov that, ecx
		
		WndConfig& cfg = WndConfig::Get();
		if (cfg.mode > WNDMODE_DEFAULT)
			d3dpp->Windowed = TRUE;

		return reinterpret_cast<bool(__thiscall*)(uintptr_t, D3DPRESENT_PARAMETERS*, uint32_t)>(p_CreateD3DDevice)(that, d3dpp, creationFlags);
	}

	//uintptr_t p_CWnd_ShowWindow;
	//static void CWnd_ShowWindow(uintptr_t cwnd, int showWnd)
	//{
	//	return reinterpret_cast<void(__thiscall*)(uintptr_t, int)>(p_CWnd_ShowWindow)(cwnd, showWnd);
	//}


	uintptr_t p_CWnd_CreateEx;
	static BOOL _stdcall CWnd_CreateEx_hook(DWORD dwExStyle,
		LPCSTR lpszClassName,
		LPCSTR lpszWindowName,
		DWORD dwStyle,
		const LPRECT rect,
		uintptr_t parent,
		uint32_t unk,
		LPVOID lpParam)
	{
		uintptr_t that;
		_asm mov that, ecx

		BOOL retVal = FALSE;

		SetProcessDPIAware();

		WndConfig& cfg = WndConfig::Get();

		if (cfg.mode <= WNDMODE_DEFAULT)
		{
			retVal = reinterpret_cast<BOOL(__thiscall*)(uintptr_t, DWORD, LPCSTR, LPCSTR, DWORD, const LPRECT, uintptr_t, uint32_t, LPVOID)>(p_CWnd_CreateEx)(that, dwExStyle, lpszClassName, lpszWindowName, dwStyle, rect, parent, unk, lpParam);
			if (retVal)
			{
				GameHWND = GetHWNDFromCWnd(that);
				YeetEdge();
			}
			return retVal;
		}

		retVal = reinterpret_cast<BOOL(__thiscall*)(uintptr_t, DWORD, LPCSTR, LPCSTR, DWORD, const LPRECT, uintptr_t, uint32_t, LPVOID)>(p_CWnd_CreateEx)(that, dwExStyle, lpszClassName, lpszWindowName, dwStyle, rect, parent, unk, lpParam);

		if (retVal)
		{
			GameHWND = GetHWNDFromCWnd(that);
			//printf("style: 0x%X\n", GetWindowLong(GameHWND, GWL_STYLE));
			//printf("exstyle: 0x%X\n", GetWindowLong(GameHWND, GWL_EXSTYLE));
			afterCreateWindow();
		}

		return retVal;
	}

	uintptr_t p_CWnd_PreCreateWindow;
	static BOOL __stdcall CWnd_PreCreateWindow_hook(CREATESTRUCT& cs)
	{
		uintptr_t that;
		_asm mov that, ecx

		cs.style = GetDesiredWindowStyle();
		cs.dwExStyle = GetDesiredWindowExStyle();

		auto [WindowPosX, WindowPosY, newWidth, newHeight] = beforeCreateWindow(cs.cx, cs.cy);
		cs.x = WindowPosX;
		cs.y = WindowPosY;

		RECT rc = { WindowPosX, WindowPosY, newWidth + WindowPosX, newHeight + WindowPosY };
		AdjustWindowRectEx(&rc, cs.style, FALSE, cs.dwExStyle | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE); // WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE ARE NEEDED HERE FOR CALCULATIONS (we'll yeet them afterwards!) THANK YOU EWWTECHNYX FOR USING MFC!!!
		cs.cx = rc.right - rc.left;
		cs.cy = rc.bottom - rc.top;


		return reinterpret_cast<BOOL(__thiscall*)(uintptr_t, CREATESTRUCT&)>(p_CWnd_PreCreateWindow)(that, cs);
	}


	//uintptr_t p_CWnd_Default;
	//static void CWnd_Default(uintptr_t cwnd)
	//{
	//	reinterpret_cast<void(__thiscall*)(uintptr_t)>(p_CWnd_Default)(cwnd);
	//}

	static void __stdcall CWnd_Default_hook()
	{
		// nuked this function -- the game calls it EVERY FRAME?!?! WHY?!

		//uintptr_t that;
		//_asm mov that, ecx
		//CWnd_Default(that);
		//afterCreateWindow();
	}

	//uintptr_t p_CWnd_Default_1;
	//uintptr_t p_CWnd_Default_2;
	//
	//static void __stdcall CWnd_Default_hook_OnWindowCreate(uintptr_t that, uintptr_t pFunc)
	//{
	//	afterCreateWindow();
	//}
	//
	//static void __stdcall CWnd_Default_hook_OnWindowCreate1()
	//{
	//	uintptr_t that;
	//	_asm mov that, ecx
	//	return CWnd_Default_hook_OnWindowCreate(that, p_CWnd_Default_1);
	//}
	//
	//static void __stdcall CWnd_Default_hook_OnWindowCreate2()
	//{
	//	uintptr_t that;
	//	_asm mov that, ecx
	//	return CWnd_Default_hook_OnWindowCreate(that, p_CWnd_Default_2);
	//}



	//void __declspec(naked) hkCWndCreateEx()
	//{
	//	_asm
	//	{
	//		mov ebp, esp
	//		push [ebp + 0x20]
	//		push [ebp + 0x1C]
	//		push [ebp + 0x18]
	//		push [ebp + 0x14]
	//		push [ebp + 0x10]
	//		push [ebp + 0xC]
	//		push [ebp + 8]
	//		push [ebp + 4]
	//		call CreateEx_hook
	//		test eax, eax
	//		retn 0x20
	//	}
	//}

	//bool bSendMessageOnceFlag = false;

	void __stdcall SomethingOnSendMessage(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
	{
		// nuked this function -- calls it every frame for no good reason

		//uintptr_t that;
		//_asm mov that, ecx
		////
		////CWnd_ShowWindow(that, 9);
		//
		//if (!bSendMessageOnceFlag)
		//{
		//	bSendMessageOnceFlag = true;
		//	afterCreateWindow();
		//}
	}

#pragma runtime_checks( "", restore )

	static uintptr_t GetBranchDestination_FarJmp(uintptr_t at)
	{
		return **reinterpret_cast<uintptr_t**>(at + 2);
	}

	void Init(WindowedModes mode)
	{
		// #TODO individual hooks
		uintptr_t loc_4CE350 = 0x4CE350;
		uintptr_t loc_4CE361 = 0x4CE361;
		uintptr_t loc_4CE36F = 0x4CE36F;

		p_CreateD3DDevice = static_cast<uintptr_t>(injector::GetBranchDestination(loc_4CE350));

		injector::MakeCALL(loc_4CE350, CreateD3DDevice_Hook);
		injector::MakeCALL(loc_4CE361, CreateD3DDevice_Hook);
		injector::MakeCALL(loc_4CE36F, CreateD3DDevice_Hook);

		//uintptr_t loc_403D16 = 0x403D16;
		//injector::MakeCALL(loc_403D16, hkCWndCreateEx);

		constexpr int vtidx_CreateEx = 24;
		constexpr int vtidx_PreCreateWindow = 27;
		constexpr int vtidx_CalcWindowRect = 28;
		uintptr_t* Wnd_vftable = reinterpret_cast<uintptr_t*>(0x712028);

		p_CWnd_CreateEx = Wnd_vftable[vtidx_CreateEx];
		injector::WriteMemory(&Wnd_vftable[vtidx_CreateEx], &CWnd_CreateEx_hook, true);

		p_CWnd_PreCreateWindow = Wnd_vftable[vtidx_PreCreateWindow];
		injector::WriteMemory(&Wnd_vftable[vtidx_PreCreateWindow], &CWnd_PreCreateWindow_hook, true);

		//p_CalcWindowRect = Wnd_vftable[vtidx_CalcWindowRect];
		//injector::WriteMemory(&Wnd_vftable[vtidx_CalcWindowRect], &CalcWindowRect_hook, true);


		// ewwwtechnyx, why do you launch the game minimized??? that makes no sense!
		injector::MakeJMP(0x403D88, 0x403DA2);

		uintptr_t loc_640FC6 = 0x640FC6;
		//p_CWnd_Default = GetBranchDestination_FarJmp(loc_640FC6);
		injector::MakeJMP(loc_640FC6, CWnd_Default_hook);

		//uintptr_t loc_403DB9 = 0x403DB9;
		//p_CWnd_ShowWindow = static_cast<uintptr_t>(injector::GetBranchDestination(loc_403DB9));

		injector::MakeJMP(0x403D40, SomethingOnSendMessage);

		//uintptr_t loc_403DA4 = 0x403DA4;
		//p_CWnd_Default_1 = static_cast<uintptr_t>(injector::GetBranchDestination(loc_403DA4));
		//injector::MakeCALL(loc_403DA4, CWnd_Default_hook_OnWindowCreate1);

		//uintptr_t loc_403DC5 = 0x403DC5;
		//p_CWnd_Default_2 = static_cast<uintptr_t>(injector::GetBranchDestination(loc_403DC5));
		//injector::MakeCALL(loc_403DC5, CWnd_Default_hook_OnWindowCreate2);

		WndConfig& cfg = WndConfig::Get();
		cfg.mode = mode;
		switch (mode)
		{
			case WNDMODE_BORDERLESS_STRETCH:
				cfg.bStretchWindow = true;
				break;
			case WNDMODE_BORDERLESS:
				cfg.bScaleWindow = true;
				break;
			case WNDMODE_BORDERED:
				cfg.bEnableWindowResize = true;
				cfg.bBorderlessWindowed = false;
				break;
			default:
				break;
		}

	}
}
