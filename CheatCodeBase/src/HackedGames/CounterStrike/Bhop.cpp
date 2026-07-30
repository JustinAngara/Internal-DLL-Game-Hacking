#include "Bhop.h"
#include <Windows.h>

void Bhop::run()
{
	UINT lpOffset{ 0x100 };
	UINT fFlag{ 0x2D };
	uintptr_t module{ (DWORD)GetModuleHandleA("client.dll") };
	uintptr_t lp{ *(DWORD*)(module + lpOffset) };

	UINT flag{ *(BYTE*)(lp+fFlag) };
	if (GetAsyncKeyState(VK_SPACE) && flag & (1 << 0))
	{
		*(DWORD*)(module + fFlag) = 6;
	}
}