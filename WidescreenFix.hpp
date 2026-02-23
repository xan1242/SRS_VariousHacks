#pragma once
#ifndef WIDESCREENFIX_HPP
#define WIDESCREENFIX_HPP

namespace WidescreenFix
{
	void SetRes(const uint32_t width, const uint32_t height);
	void SetFixHUD(bool state);
	void SetFixFOV(bool state);

	void Init();
}

#endif