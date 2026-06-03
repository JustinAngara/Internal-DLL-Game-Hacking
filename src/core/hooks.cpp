#include "Hooks.h"
#include <iostream>
typedef BOOL(__stdcall* twglSwapBuffers) (HDC hdc);
twglSwapBuffers owglSwapBuffers;

BOOL __stdcall hkwglSwapBuffers(HDC hdc)
{
	return owglSwapBuffers(hdc);
}


// trampoline hooking
void Hooks::Setup()
{

	std::cout << "attached to method\n";

	
}