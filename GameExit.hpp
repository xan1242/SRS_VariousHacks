#pragma once
#include "framework.h"

#ifndef GAMEEXIT_HPP
#define GAMEEXIT_HPP

namespace GameExit
{
	// Exit the game gracefully...
	void Exit();
	bool IsExitState();

	// #TODO: enable this when needed
	//namespace Walker
	//{
	//	void Add(void(*fn)(void*), void* param = nullptr);
	//}

	void Init();
}

#endif