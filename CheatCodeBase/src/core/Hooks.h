#pragma once
#include <windows.h>
namespace Hooks
{
	void Setup();
	bool DetourHook(void* toHook, void* ourFunc, int len);
}