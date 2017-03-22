#pragma once
#include <g2dinput.h>
#include <gmlrect.h>

constexpr uint32_t PRESSING_INTERVAL = 700u;

class KeyButtonState
{
public:
	bool isPressing = false;
	uint32_t pressTimeStamp;
};