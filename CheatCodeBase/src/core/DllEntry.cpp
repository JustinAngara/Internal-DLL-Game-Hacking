#include <Windows.h>
#include <stdexcept>
#include <stdio.h>
#include <d3d11.h>
#include "Gui/gui.h"
#include "ext/minhook/MinHook.h"
#include "sdk/Test/Test.h"
#include "Obfuscation/SpoofReturnAddress/SpoofRet.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

Gui g_gui;

using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
PresentFn o_present = nullptr;

//HRESULT WINAPI HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
//{
//	if (!g_gui.m_setup)
//		g_gui.setupMenu(swapChain);
//
//	g_gui.render();
//
//	return o_present(swapChain, syncInterval, flags);
//}


void foo()
{
	int a = 5;
	int b = 6;
	void* ret = _ReturnAddress();

	printf("ret = 0x%p\n", ret);

	printf("a + b = %i\n", (a + b));
}
static void MainThread(HMODULE hModule)
{
	AllocConsole();
	FILE* console;
	freopen_s(&console, "CONOUT$", "w", stdout);


	printf("Run() called\n"); // now visible
	//Test::Obfuscation::Run();
	//SpoofRet::Run();
	SpoofRet r;
	r.Run(foo);

	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(
			nullptr, 0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread),
			hModule, 0, nullptr)
		);
	}
	return TRUE;
}