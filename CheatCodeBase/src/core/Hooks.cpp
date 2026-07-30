#include "Hooks.h"
#include <iostream>
typedef BOOL(__stdcall* twglSwapBuffers) (HDC hdc);
twglSwapBuffers owglSwapBuffers;

BOOL __stdcall hkwglSwapBuffers(HDC hdc)
{
	return owglSwapBuffers(hdc);
}


// trampoline hooking
// implementation coming soon for min hook
void Hooks::Setup()
{	
}



// detouring hooking
bool Hooks::DetourHook(void* toHook, void* ourFunc, int len)
{
	if (len < 5)
	{
		return false;
	}

	DWORD currentProt;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &currentProt);
	memset(toHook, 0x90, len); // nop
	DWORD relativeAddr{ (DWORD)ourFunc - (DWORD)toHook - 5 };
	*(BYTE*)toHook = 0xE9; // jmp
	*(DWORD*)((DWORD)toHook + 1) = relativeAddr;

	DWORD temp;
	VirtualProtect(toHook, len, currentProt, &temp);

	return true;
}


#if defined(_M_IX86)
	DWORD jmpBackAddy;
	void __declspec(naked) ourFunc()
	{
		__asm
		{
			jmp[jmpBackAddy]
		}
	}
#endif