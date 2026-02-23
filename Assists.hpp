#pragma once
#include "framework.h"

#ifndef ASSISTS_HPP
#define ASSISTS_HPP

namespace Assists
{
	void SetABS(bool state);
	bool GetABS();

	namespace Steering
	{
		void SetYawSteerAssist(float val);
		float GetYawSteerAssist();

		void SetAssistWeight(float val);
		float GetAssistWeight();

		void SetAssistType(SteeringAssistType type);
		SteeringAssistType GetAssistType();
	}

	void Init();
}

#endif