#include "framework.h"
#include "WidescreenFix.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

#include "includes/injector/injector.hpp"
#include "includes/injector/assembly.hpp"
#include <tuple>
#include <vector>

namespace WidescreenFix
{
    bool bFixHUD;
    bool bFixFOV;

    struct Screen
    {
        uint32_t Width;
        uint32_t Height;
        float fWidth;
        float fHeight;
        float fFieldOfView;
        float fAspectRatio;
        uint32_t Width43;
        float fWidth43;
        float fHudScale;

        float GetFOV2(float f, float ar)
        {
            return 4.0f * f * atan(ar * (3.0f / 4.0f)) / (float)M_PI;
        }

        std::tuple<int32_t, int32_t> GetDesktopRes()
        {
            HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
            MONITORINFO info = {};
            info.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &info);
            int32_t DesktopResW = info.rcMonitor.right - info.rcMonitor.left;
            int32_t DesktopResH = info.rcMonitor.bottom - info.rcMonitor.top;
            return std::make_tuple(DesktopResW, DesktopResH);
        }

        void SetRes(const uint32_t width, const uint32_t height)
        {
            Width = width;
            Height = height;

            if (!Width || !Height)
                std::tie(Width, Height) = GetDesktopRes();

            fWidth = static_cast<float>(Width);
            fHeight = static_cast<float>(Height);
            fAspectRatio = (fWidth / fHeight);
            Width43 = static_cast<uint32_t>(fHeight * (4.0f / 3.0f));
            fWidth43 = static_cast<float>(Width43);
            fHudScale = fAspectRatio / (4.0f / 3.0f);
            fFieldOfView = GetFOV2(0.55f, fAspectRatio);
        }
    } Screen;


    void SetRes(const uint32_t width, const uint32_t height)
    {
        Screen.SetRes(width, height);
    }

    void SetFixHUD(bool state)
    {
        bFixHUD = state;
    }

    void SetFixFOV(bool state)
    {
        bFixFOV = state;
    }

    // #TODO: make getters...
    // #TODO make this suck less... everything is static and yucky, copied mostly from ws fix code... made the addresses static for now because the pattern detectors and labelling were HORRIBLE

	void Init()
	{
        uintptr_t loc_4C81A3 = 0x4C81A3;
        uintptr_t loc_4C81AA = 0x4C81AA;

        injector::WriteMemory(loc_4C81A3 + 3, Screen.fWidth, true);
        injector::WriteMemory(loc_4C81AA + 3, Screen.fWidth, true);

        uintptr_t loc_4C8235 = 0x4C8235;
        uintptr_t loc_4C8238 = loc_4C8235 + 3;
        uintptr_t loc_4C824E = loc_4C8238 + 0x16;

        struct SetResHook
        {
            void operator()(injector::reg_pack& regs)
            {
                float temp = 0.0f;
                _asm {fstp    dword ptr[temp]}

                *(float*)(regs.edi + 0x14) = Screen.fWidth;
                *(float*)(regs.edi + 0x18) = Screen.fHeight;
            }
        }; injector::MakeInline<SetResHook>(loc_4C8238, loc_4C824E);

        //3D
        static float f640Scale = (480.0f * Screen.fAspectRatio);
        static float f3DScale = 1.0f / f640Scale;
        
        uintptr_t loc_4BC66B = 0x4BC66B;
        injector::WriteMemory(loc_4BC66B + 2, &f3DScale, true);

        // #TODO: put a pattern detector that doesn't suck (everywhere in code)
        std::vector<uintptr_t> addrs;
        addrs.push_back(0x553DFC);
        addrs.push_back(0x553E1A);
        addrs.push_back(0x553E57);
        addrs.push_back(0x5562B3);
        addrs.push_back(0x567B5C);

        for (uintptr_t& a : addrs)
        {
            injector::WriteMemory(a + 6, f640Scale, true);
        }
        
        addrs.clear();
        addrs.push_back(0x4C80BD);
        addrs.push_back(0x4CA7E6);
        addrs.push_back(0x4EC99C);
        addrs.push_back(0x4ECD22);
        
        for (uintptr_t& a : addrs)
        {
            injector::WriteMemory(a + 6, f640Scale, true);
        }
        
        
        if (bFixHUD)
        {
            Screen.fHudScale = Screen.fAspectRatio / (4.0f / 3.0f);
            uintptr_t loc_4CB3C5 = 0x4CB3C5;
            struct HudScaleHook
            {
                void operator()(injector::reg_pack& regs)
                {
                    if (*(float*)(*(uintptr_t*)(regs.ecx + 0x0A859C) + 0x08) != 90.0f && *(uintptr_t*)(*(uintptr_t*)(regs.ecx + 0x0A859C) + 0x18) != 235587392) //fading check
                    {
                        *(float*)(*(uintptr_t*)(regs.ecx + 0x0A859C) + 0x00) /= Screen.fHudScale;
                    }
        
                    regs.eax = *(uintptr_t*)(regs.ecx + 0x0C3B40);
                }
            }; injector::MakeInline<HudScaleHook>(loc_4CB3C5, loc_4CB3C5 + 6);
        
            uintptr_t loc_5288D2 = 0x5288D2;
            struct RearviewMirrorHook
            {
                void operator()(injector::reg_pack& regs)
                {
                    uint32_t fMirrorOffset = *(uint32_t*)(regs.esp + 0x8) + (uint32_t)(((480.0f * Screen.fAspectRatio) - 640.0f) / 2.0f);
                    _asm {fild    dword ptr[fMirrorOffset]}
                    regs.eax = regs.esi + 0xA8;
                }
            }; injector::MakeInline<RearviewMirrorHook>(loc_5288D2, loc_5288D2 + 0xA);
        
        }
        
        if (bFixFOV)
        {
            uintptr_t loc_4BC4BB = 0x4BC4BB;
            injector::WriteMemory(loc_4BC4BB + 2, &Screen.fFieldOfView, true);
        }
	}
}