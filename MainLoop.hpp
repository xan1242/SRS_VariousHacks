#pragma once
#include "framework.h"

#ifndef MAINLOOP_HPP
#define MAINLOOP_HPP

namespace MainLoop
{
	namespace Walker
	{
		void AddToLoop(void(*fn)(void* that, void* param), void* param = nullptr);
		void ExecuteOnce(void(*fn)(void* that, void* param), void* param = nullptr);
	}

	void Init();
}

#endif