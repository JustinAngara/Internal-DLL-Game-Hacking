#pragma once
#include <d3d11.h>  
#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_dx11.h"  
class Gui
{
public:
    bool setupWindowClass(const wchar_t* windowClassName) noexcept;
    void destroyWindowClass() noexcept;

    bool setupWindow(const wchar_t* windowName) noexcept;
    void destroyWindow() noexcept;

    bool setupDirectX() noexcept;
    void destroyDirectX() noexcept;

    void setup();
    
    void setupMenu(IDXGISwapChain* swapChain) noexcept;
    void destroy() noexcept;

    void render() noexcept;

private:
    bool m_open = true;
    bool m_setup = false;

    HWND m_window = nullptr;
    WNDCLASSEX m_windowClass = { };
    WNDPROC m_originalWindowProcess = nullptr;

    ID3D11DeviceContext* m_deviceContext = nullptr;
    ID3D11Device* m_device = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
};