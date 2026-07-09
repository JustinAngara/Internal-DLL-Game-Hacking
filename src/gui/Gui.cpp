#include "gui.h"

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_win32.h"
#include "ext/imgui/imgui_impl_dx11.h"

#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib") 

WNDPROC g_originalWindowProcess = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window,
    UINT message,
    WPARAM windowParam,
    LPARAM longParam
);


LRESULT CALLBACK WindowProcess(
    HWND window,
    UINT message,
    WPARAM windowParam,
    LPARAM longParam
)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, windowParam, longParam))
        return true;

    return CallWindowProc(
        g_originalWindowProcess, 
        window,
        message,
        windowParam,
        longParam
    );
}


bool Gui::setupWindowClass(const wchar_t* windowClassName) noexcept
{
    m_windowClass.cbSize        = sizeof(WNDCLASSEX);
    m_windowClass.style         = CS_HREDRAW | CS_VREDRAW;
    m_windowClass.lpfnWndProc   = DefWindowProc;
    m_windowClass.cbClsExtra    = 0;
    m_windowClass.cbWndExtra    = 0;
    m_windowClass.hInstance     = GetModuleHandle(NULL);
    m_windowClass.hIcon         = NULL;
    m_windowClass.hCursor       = NULL;
    m_windowClass.hbrBackground = NULL;
    m_windowClass.lpszMenuName  = NULL;
    m_windowClass.lpszClassName = windowClassName;
    m_windowClass.hIconSm       = NULL;

    if (!RegisterClassEx(&m_windowClass))
        return false;

    return true;
}

void Gui::destroyWindowClass() noexcept
{
    UnregisterClass(
        m_windowClass.lpszClassName,
        m_windowClass.hInstance
    );
}

bool Gui::setupWindow(const wchar_t* windowName) noexcept
{
    m_window = CreateWindow(
        m_windowClass.lpszClassName,
        windowName,
        WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100,
        0, 0,
        m_windowClass.hInstance,
        0
    );

    if (!m_window)
        return false;

    return true;
}

void Gui::destroyWindow() noexcept
{
    if (m_window)
        ::DestroyWindow(m_window); 
}

bool Gui::setupDirectX() noexcept
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc      = {};
    swapChainDesc.BufferCount               = 1;
    swapChainDesc.BufferDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow              = m_window;
    swapChainDesc.SampleDesc.Count          = 1;
    swapChainDesc.Windowed                  = TRUE;
    swapChainDesc.SwapEffect                = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_deviceContext
    ))) return false;

    return true;
}

void Gui::destroyDirectX() noexcept
{
    if (m_deviceContext) { m_deviceContext->Release(); m_deviceContext = nullptr; }
    if (m_device)        { m_device->Release();        m_device        = nullptr; }
    if (m_swapChain)     { m_swapChain->Release();     m_swapChain     = nullptr; }
}

void Gui::setup()
{
    if (!setupWindowClass(L"GUI"))
        throw std::runtime_error("Failed to create window class.");

    if (!setupWindow(L"GUI"))
        throw std::runtime_error("Failed to create window.");

    if (!setupDirectX())
        throw std::runtime_error("Failed to create device.");

    destroyWindow();
    destroyWindowClass();
}
void Gui::setupMenu(IDXGISwapChain* swapChain) noexcept
{
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device),
        reinterpret_cast<void**>(&m_device))))
        return;

    m_device->GetImmediateContext(&m_deviceContext);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChain->GetDesc(&swapChainDesc);
    m_window = swapChainDesc.OutputWindow;

    g_originalWindowProcess = m_originalWindowProcess = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
    );

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(m_window);
    ImGui_ImplDX11_Init(m_device, m_deviceContext);

    m_setup = true;
}

void Gui::destroy() noexcept
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    SetWindowLongPtr(
        m_window,
        GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(m_originalWindowProcess)
    );

    destroyDirectX();
}

void Gui::render() noexcept
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("amazing software", &m_open);
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}