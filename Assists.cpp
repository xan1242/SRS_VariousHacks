#include "framework.h"
#include "Assists.hpp"
#include "includes/injector/injector.hpp"


namespace Assists
{
	bool bABS = true;

	void SetABS(bool state)
	{
		bABS = state;
	}

	bool GetABS()
	{
		return bABS;
	}

	uintptr_t p_uProcABS;

#pragma runtime_checks( "", off )

	static void __stdcall uProcABS_Hook(uintptr_t arg)
	{
		uintptr_t that;
		_asm mov that, ecx

		if (!bABS)
			return;

		reinterpret_cast<void(__thiscall*)(uintptr_t, uintptr_t)>(p_uProcABS)(that, arg);
	}

#pragma runtime_checks( "", restore )

	namespace Steering
	{
		float YawSteerAssist = -1.0f;
		float AssistWeight = -1.0f;
		float ReturnSpring = -1.0f;
		SteeringAssistType AssistType = STEERINGASSIST_UNK;

		void SetYawSteerAssist(float val)
		{
			YawSteerAssist = val;
		}

		float GetYawSteerAssist()
		{
			return YawSteerAssist;
		}

		void SetAssistWeight(float val)
		{
			AssistWeight = val;
		}

		float GetAssistWeight()
		{
			return AssistWeight;
		}

		void SetAssistType(SteeringAssistType type)
		{
			AssistType = type;
		}

		float GetReturnSpring()
		{
			return ReturnSpring;
		}

		void SetReturnSpring(float val)
		{
			ReturnSpring = val;
		}

		SteeringAssistType GetAssistType()
		{
			return AssistType;
		}

#pragma runtime_checks( "", off )
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

		uintptr_t p_ParseFloatToken_ReturnSpring;
		float __stdcall ParseFloatToken_ReturnSpring()
		{
			uintptr_t that;
			_asm mov that, ecx

			float retVal = reinterpret_cast<float(__thiscall*)(uintptr_t)>(p_ParseFloatToken_ReturnSpring)(that);

			if (ReturnSpring < 0.0f)
				return retVal;

			return ReturnSpring;
		}

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

		static void Init()
		{
			uintptr_t loc_6E5B43 = 0x6E5B43;
			p_ParseFloatToken_YawSteerAssist = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5B43));
			injector::MakeCALL(loc_6E5B43, ParseFloatToken_YawSteerAssist);

			uintptr_t loc_6E5B63 = 0x6E5B63;
			p_ParseFloatToken_ReturnSpring = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5B63));
			injector::MakeCALL(loc_6E5B63, ParseFloatToken_ReturnSpring);
			
			uintptr_t loc_6E5B23 = 0x6E5B23;
			p_ParseFloatToken_AssistWeight = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5B23));
			injector::MakeCALL(loc_6E5B23, ParseFloatToken_AssistWeight);
			
			uintptr_t loc_6E5A81 = 0x6E5A81;
			p_ParseAssistType = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6E5A81));
			injector::MakeCALL(loc_6E5A81, ParseAssistType_Hook);
		}
	}

	void Init()
	{
		Steering::Init();

		uintptr_t loc_6D0503 = 0x6D0503;
		p_uProcABS = static_cast<uintptr_t>(injector::GetBranchDestination(loc_6D0503));
		injector::MakeCALL(loc_6D0503, uProcABS_Hook);
	}
}