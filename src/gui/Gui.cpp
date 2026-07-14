#include "Gui.h"
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Stored by setupMenu, read by WindowProcess
static WNDPROC g_originalWindowProcess = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window, UINT message, WPARAM wParam, LPARAM lParam);

// Intercepts window messages so ImGui gets input
LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
		return true;

	return CallWindowProc(g_originalWindowProcess, window, message, wParam, lParam);
}

// ------------------------------------------------------------------ internals

bool Gui::setupWindowClass(const wchar_t* name) noexcept
{
	m_windowClass         = {};
	m_windowClass.cbSize  = sizeof(WNDCLASSEX);
	m_windowClass.style   = CS_HREDRAW | CS_VREDRAW;
	m_windowClass.lpfnWndProc   = DefWindowProc;
	m_windowClass.hInstance     = GetModuleHandle(nullptr);
	m_windowClass.lpszClassName = name;
	return RegisterClassEx(&m_windowClass) != 0;
}

void Gui::destroyWindowClass() noexcept
{
	UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);
}

bool Gui::setupWindow(const wchar_t* name) noexcept
{
	m_window = CreateWindowW(
		m_windowClass.lpszClassName, name,
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		nullptr, nullptr, m_windowClass.hInstance, nullptr);
	return m_window != nullptr;
}

void Gui::destroyWindow() noexcept
{
	if (m_window) { ::DestroyWindow(m_window); m_window = nullptr; }
}

bool Gui::setupDirectX() noexcept
{
	DXGI_SWAP_CHAIN_DESC sd  = {};
	sd.BufferCount           = 1;
	sd.BufferDesc.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow          = m_window;
	sd.SampleDesc.Count      = 1;
	sd.Windowed              = TRUE;
	sd.SwapEffect            = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL fl[]   = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL flOut;

	return SUCCEEDED(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		fl, 1, D3D11_SDK_VERSION, &sd,
		&m_swapChain, &m_device, &flOut, &m_deviceContext));
}

void Gui::destroyDirectX() noexcept
{
	if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
	if (m_deviceContext)    { m_deviceContext->Release();    m_deviceContext    = nullptr; }
	if (m_swapChain)        { m_swapChain->Release();        m_swapChain        = nullptr; }
	if (m_device)           { m_device->Release();           m_device           = nullptr; }
}

void** Gui::getSwapChainVTable() noexcept
{
	if (!m_swapChain) return nullptr;
	return *reinterpret_cast<void***>(m_swapChain);
}

// ------------------------------------------------------------------ public API

void Gui::setup()
{
	if (!setupWindowClass(L"DX11Hook"))
		throw std::runtime_error("Window class failed");
	if (!setupWindow(L"DX11Hook"))
		throw std::runtime_error("Window failed");
	if (!setupDirectX())
		throw std::runtime_error("DirectX failed");

	// Window is only needed to create the device — destroy it now
	// but leave the DX objects alive so MainThread can read the vtable
	destroyWindow();
	destroyWindowClass();
}

void Gui::setupMenu(IDXGISwapChain* swapChain) noexcept
{
	// Get the real device from the game's swap chain
	if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device),
		reinterpret_cast<void**>(&m_device))))
		return;

	m_device->GetImmediateContext(&m_deviceContext);

	// Build a render target from the game's back buffer
	ID3D11Texture2D* backBuffer = nullptr;
	if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&backBuffer))))
	{
		m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
		backBuffer->Release();
	}

	// Get the game window from the swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd;
	swapChain->GetDesc(&sd);
	m_window = sd.OutputWindow;

	// Hijack the game's WNDPROC so ImGui receives input
	g_originalWindowProcess = m_originalWindowProcess =
		reinterpret_cast<WNDPROC>(SetWindowLongPtr(
			m_window, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(WindowProcess)));

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Disable ImGui saving layout to disk (optional but clean)
	ImGui::GetIO().IniFilename = nullptr;

	ImGui_ImplWin32_Init(m_window);
	ImGui_ImplDX11_Init(m_device, m_deviceContext);

	m_setup = true;
}

void Gui::render() noexcept
{
	// Toggle with Insert
	if (GetAsyncKeyState(VK_INSERT) & 1)
		m_open = !m_open;

	// Always bind render target and run full ImGui frame
	// Skipping EndFrame/Render corrupts ImGui's state machine
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (m_open)
	{
		ImGui::Begin("Menu", &m_open);
		ImGui::Text("Working!");
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Gui::destroy() noexcept
{
	if (m_setup)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		// Restore the original WNDPROC
		SetWindowLongPtr(m_window, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(m_originalWindowProcess));

		m_setup = false;
	}

	destroyDirectX();
}