#include <Windows.h>
#include <stdexcept>
#include <stdio.h>
#include <d3d11.h>
#include "Gui/gui.h"
#include "ext/minhook/MinHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Single global instance — defined here, extern'd anywhere else that needs it
Gui g_gui;

using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
PresentFn o_present = nullptr;

HRESULT WINAPI HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
	if (!g_gui.m_setup)
		g_gui.setupMenu(swapChain);

	g_gui.render();

	return o_present(swapChain, syncInterval, flags);
}

static void MainThread(HMODULE hModule)
{
	// ---- 1. Create dummy window + DX11 device to steal the vtable ----
	try { g_gui.setup(); }
	catch (const std::exception& e)
	{
		MessageBoxA(0, e.what(), "setup() failed", MB_OK);
		FreeLibraryAndExitThread(hModule, 0);
		return;
	}

	// ---- 2. Get Present's address from the dummy vtable ----
	// IDXGISwapChain vtable layout (slot index):
	//   0  QueryInterface
	//   1  AddRef
	//   2  Release
	//   3  SetPrivateData
	//   4  SetPrivateDataInterface
	//   5  GetPrivateData
	//   6  GetParent
	//   7  GetDevice
	//   8  Present        <-- this is what we want
	void** vtable = g_gui.getSwapChainVTable();
	if (!vtable)
	{
		MessageBoxA(0, "vtable is null — swap chain was destroyed too early", "ERROR", MB_OK);
		FreeLibraryAndExitThread(hModule, 0);
		return;
	}

	void* presentAddr = vtable[8];

	// ---- 3. Release the dummy DX objects — we only needed the address ----
	g_gui.destroyDirectX();

	// ---- 4. Hook Present using MinHook ----
	// MinHook patches the function's first bytes (not the vtable)
	// so it intercepts ALL callers, not just our dummy swap chain
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(0, "MH_Initialize failed", "ERROR", MB_OK);
		FreeLibraryAndExitThread(hModule, 0);
		return;
	}

	MH_STATUS status = MH_CreateHook(
		presentAddr,
		&HookedPresent,
		reinterpret_cast<void**>(&o_present));

	if (status != MH_OK)
	{
		char buf[64];
		sprintf_s(buf, "MH_CreateHook failed: %d", (int)status);
		MessageBoxA(0, buf, "ERROR", MB_OK);
		MH_Uninitialize();
		FreeLibraryAndExitThread(hModule, 0);
		return;
	}

	MH_EnableHook(presentAddr);

	// ---- 5. Stay alive until End key is pressed ----
	while (!GetAsyncKeyState(VK_END))
		Sleep(100);

	// ---- 6. Cleanup ----
	MH_DisableHook(presentAddr);
	MH_Uninitialize();
	g_gui.destroy();
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
			hModule, 0, nullptr));
	}
	return TRUE;
}