#include "framework.h"
#include "MainLoop.hpp"
#include "includes/injector/injector.hpp"

#include <functional>
#include <deque>
#include <vector>

namespace MainLoop
{
	uintptr_t p_MainLoop;

#pragma runtime_checks( "", off )

	namespace FuncWalker
	{
		static void HandleLoop(void* that, std::vector<std::pair<void*, void(*)(void*, void*)>>& sFunctions, std::deque<std::pair<void*, void(*)(void*, void*)>>& sQueue)
		{
			for (const auto& fn : sFunctions) fn.second(that, fn.first);

			while (!sQueue.empty())
			{
				sQueue.front().second(that, sQueue.front().first);
				sQueue.pop_front();
			}
		}
	}

	namespace Walker
	{
		static std::vector<std::pair<void*, void(*)(void*, void*)>> sFunctions;
		static std::deque<std::pair<void*, void(*)(void*, void*)>>  sQueue;

		void AddToLoop(void(*fn)(void* that, void* param), void* param)
		{
			sFunctions.push_back(std::make_pair(param, fn));
		}

		void ExecuteOnce(void(*fn)(void* that, void* param), void* param)
		{
			sQueue.push_back(std::make_pair(param, fn));
		}
	}

	static bool __stdcall MainLoop_Hook()
	{
		void* that;
		_asm mov that, ecx

		FuncWalker::HandleLoop(that, Walker::sFunctions, Walker::sQueue);

		return reinterpret_cast<bool(__thiscall*)(void*)>(p_MainLoop)(that);
	}

#pragma runtime_checks( "", restore )

	void Init()
	{
		uintptr_t loc_4053F5 = 0x4053F5;
		p_MainLoop = static_cast<uintptr_t>(injector::GetBranchDestination(loc_4053F5));
		injector::MakeCALL(loc_4053F5, MainLoop_Hook);
	}
}