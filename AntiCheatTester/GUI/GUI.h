#pragma once
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx11.h"
#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>

namespace GUI
{
    namespace Config
    {
        constexpr const wchar_t* Title = L"Anti-Cheat Tester";

        constexpr int X      = 100;
        constexpr int Y      = 100;
        constexpr int Width  = 500;
        constexpr int Height = 800;

        constexpr bool IsResizable = true;
        constexpr bool HasBorder   = true;
        constexpr bool CloseOnExit = true;

        // RGBA behind content
        inline float ClearColor[4] = { 0.10f, 0.10f, 0.10f, 1.00f };

        // direct3d state
        inline ID3D11Device*           g_pd3dDevice           = nullptr;
        inline ID3D11DeviceContext*    g_pd3dDeviceContext    = nullptr;
        inline IDXGISwapChain*         g_pSwapChain           = nullptr;
        inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

        // window state
        inline HWND        g_hwnd = nullptr;
        inline WNDCLASSEXW g_wc   = {};
        inline UINT        g_ResizeWidth = 0, g_ResizeHeight = 0;
    }

    // creates, renders, exits
    void Run();
}