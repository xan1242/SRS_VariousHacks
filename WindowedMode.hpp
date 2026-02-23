#pragma once
#include "framework.h"

#ifndef WINDOWEDMODE_HPP
#define WINDOWEDMODE_HPP

namespace WindowedMode
{
    struct WndConfig
    {
        bool bBorderlessWindowed = true;
        bool bEnableWindowResize = false;
        bool bScaleWindow = false;
        bool bStretchWindow = false;
        bool bAlwaysOnTop = false;

        WindowedModes mode = WNDMODE_UNK;

        static WndConfig& Get()
        {
            static WndConfig sConfig;
            return sConfig;
        }
    };

    void Init(WindowedModes mode);
}

#endif