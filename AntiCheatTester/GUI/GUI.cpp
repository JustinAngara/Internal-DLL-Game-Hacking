#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx11.h"
#include "GUI.h"
#include <string>
#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
namespace GC = GUI::Config;

namespace
{
    void DrawUI(); // annoying dependency call but want to restructure
    void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer = nullptr;
        GC::g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        GC::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &GC::g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void CleanupRenderTarget()
    {
        if (GC::g_mainRenderTargetView)
        {
            GC::g_mainRenderTargetView->Release();
            GC::g_mainRenderTargetView = nullptr;
        }
    }

    bool CreateDeviceD3D(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount                        = 2;
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = hWnd;
        sd.SampleDesc.Count                   = 1;
        sd.Windowed                           = TRUE;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            levels, 2, D3D11_SDK_VERSION, &sd, &GC::g_pSwapChain,
            &GC::g_pd3dDevice, &featureLevel, &GC::g_pd3dDeviceContext) != S_OK)
        {
            return false;
        }

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (GC::g_pSwapChain)        { GC::g_pSwapChain->Release();        GC::g_pSwapChain = nullptr; }
        if (GC::g_pd3dDeviceContext) { GC::g_pd3dDeviceContext->Release(); GC::g_pd3dDeviceContext = nullptr; }
        if (GC::g_pd3dDevice)        { GC::g_pd3dDevice->Release();        GC::g_pd3dDevice = nullptr; }
    }

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                return 0;
            }
            GC::g_ResizeWidth  = (UINT)LOWORD(lParam);
            GC::g_ResizeHeight = (UINT)HIWORD(lParam);

            return 0;
        case WM_DESTROY:
            if (GC::CloseOnExit)
            {
                PostQuitMessage(0);
            }
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    // build win32 style flags
    DWORD BuildWindowStyle()
    {
        if (!GC::HasBorder)
        {
            return WS_POPUP;
        }

        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!GC::IsResizable)
        {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);  // drop resize grip and maximise
        }
        return style;
    }

    // create window
    bool Setup()
    {
        GC::g_wc = { sizeof(GC::g_wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGuiClass", nullptr };
        RegisterClassExW(&GC::g_wc);

        GC::g_hwnd = CreateWindowW(GC::g_wc.lpszClassName, GC::Title, BuildWindowStyle(),
                                   GC::X, GC::Y, GC::Width, GC::Height,
                                   nullptr, nullptr, GC::g_wc.hInstance, nullptr);

        if (!GC::g_hwnd)
        {
            return false;
        }

        if (!CreateDeviceD3D(GC::g_hwnd))
        {
            return false;
        }

        ShowWindow(GC::g_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(GC::g_hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(GC::g_hwnd);
        ImGui_ImplDX11_Init(GC::g_pd3dDevice, GC::g_pd3dDeviceContext);

        return true;
    }

    void Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();

        DestroyWindow(GC::g_hwnd);
        UnregisterClassW(GC::g_wc.lpszClassName, GC::g_wc.hInstance);
        GC::g_hwnd = nullptr;
    }

    bool PumpMessages()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                return false;
            }
        }
        return true;
    }

    void HandleResize()
    {
        if (GC::g_ResizeWidth != 0 && GC::g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            GC::g_pSwapChain->ResizeBuffers(0, GC::g_ResizeWidth, GC::g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            GC::g_ResizeWidth = GC::g_ResizeHeight = 0;
            CreateRenderTarget();
        }
    }

    void RenderFrame()
    {
        HandleResize();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawUI();

        ImGui::Render();
        GC::g_pd3dDeviceContext->OMSetRenderTargets(1, &GC::g_mainRenderTargetView, nullptr);
        GC::g_pd3dDeviceContext->ClearRenderTargetView(GC::g_mainRenderTargetView, GC::ClearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        GC::g_pSwapChain->Present(1, 0);
    }

    void DrawUI()
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);

        ImGui::Begin("Main", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize   |
                     ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        static float f = 0.0f;
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("Click me"))
        {
            std::cout << "test";
        }

        ImGui::End();
    }
}

void GUI::Run()
{
    if (!Setup())
    {
        Shutdown();
        return;
    }

    while (PumpMessages())
    {
        RenderFrame();
    }

    Shutdown();
}