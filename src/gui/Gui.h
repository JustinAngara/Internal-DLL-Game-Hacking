#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_dx11.h"
#include "ext/imgui/imgui_impl_win32.h"

class Gui
{
public:
	// State — read from other TUs
	bool m_open  = true;
	bool m_setup = false;

	// Called once in MainThread — creates dummy window/device to steal vtable
	void setup();

	// Called once inside HookedPresent on the first frame
	void setupMenu(IDXGISwapChain* swapChain) noexcept;

	// Called every frame inside HookedPresent
	void render() noexcept;

	// Called on eject
	void destroy() noexcept;

	// Returns vtable of the dummy swap chain so MainThread can find Present's address
	void** getSwapChainVTable() noexcept;

	// Only called by MainThread after vtable is read
	void destroyDirectX() noexcept;

private:
	bool setupWindowClass(const wchar_t* name) noexcept;
	void destroyWindowClass()                  noexcept;
	bool setupWindow(const wchar_t* name)      noexcept;
	void destroyWindow()                       noexcept;
	bool setupDirectX()                        noexcept;

	HWND             m_window              = nullptr;
	WNDCLASSEX       m_windowClass         = {};
	WNDPROC          m_originalWindowProcess = nullptr;

	ID3D11Device*            m_device          = nullptr;
	ID3D11DeviceContext*     m_deviceContext    = nullptr;
	IDXGISwapChain*          m_swapChain        = nullptr;
	ID3D11RenderTargetView*  m_renderTargetView = nullptr;
};