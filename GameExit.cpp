#include "framework.h"
#include "GameExit.hpp"
#include "includes/injector/injector.hpp"

#include <functional>
#include <vector>

namespace GameExit
{
	//uintptr_t pAfxGetThread;
	uintptr_t pGameStateThingy;

	void Exit()
	{
		*reinterpret_cast<uint32_t*>(pGameStateThingy) = 1;
	}

	bool IsExitState()
	{
		return *reinterpret_cast<uint32_t*>(pGameStateThingy) == 1;
	}

	// #TODO: enable this when needed...

//#pragma runtime_checks( "", off )
//
//	namespace FuncWalker
//	{
//		static void HandleLoop(std::vector<std::pair<void*, void(*)(void*)>>& sFunctions)
//		{
//			for (const auto& fn : sFunctions) fn.second(fn.first);
//		}
//	}
//
//	namespace Walker
//	{
//		static std::vector<std::pair<void*, void(*)(void*)>> sFunctions;
//
//		void Add(void(*fn)(void*), void* param)
//		{
//			sFunctions.push_back(std::make_pair(param, fn));
//		}
//	}
//
//	static uintptr_t AfxGetThread_Hook()
//	{
//		uintptr_t retVal = reinterpret_cast<uintptr_t(*)()>(pAfxGetThread)();
//
//		FuncWalker::HandleLoop(Walker::sFunctions);
//
//		return retVal;
//	}
//
//#pragma runtime_checks( "", restore )

	void Init()
	{
		//uintptr_t loc_4053FE = 0x4053FE;
		//pAfxGetThread = static_cast<uintptr_t>(injector::GetBranchDestination(loc_4053FE));
		//injector::MakeCALL(loc_4053FE, AfxGetThread_Hook);

		uintptr_t loc_40454F = 0x40454F;
		pGameStateThingy = *reinterpret_cast<uintptr_t*>(loc_40454F + 2);
	}
}
