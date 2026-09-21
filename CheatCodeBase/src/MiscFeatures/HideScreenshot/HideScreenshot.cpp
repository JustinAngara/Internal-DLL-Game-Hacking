#include "HideScreenshot.h"
#include "../../ext/detours/detours.h"
#include <Windows.h>
#include <iostream>

typedef BOOL(WINAPI* BitBltPtr)(HDC, int, int, int, int, HDC, int, int, DWORD);
BitBltPtr pBitBlt = nullptr;

BOOL WINAPI HookedBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) {
    std::cout << "Hooked func\n"; // TODO: UTILIZE LOGGER HERE
    
    return pBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop); 
}




BOOL DetourCleanup() {
    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    // temporarily detatches our hook and restore
    DetourDetach(&(PVOID&)pBitBlt, MyBitBlt);

    if (DetourTransactionCommit() == NO_ERROR) {
        std::cout << "Detatched hook\n";
        return true;
    }
    return false;
}



void HideScreenshot::Run()
{
    HMODULE hGdi32 = LoadLibraryA("gdi32.dll");

    pBitBlt = reinterpret_cast<BitBltPtr>(GetProcAddress(hGdi32, "BitBlt"));


    // transaction ensures reversability
    DetourTransactionBegin();

    
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)pBitBlt, MyBitBlt);

    // commits the transaction. If there is no error our hook is not rolled back.
    if (DetourTransactionCommit() == NO_ERROR) {
        std::cout << "Attached hook\n";
        return;
    }

    // figure out a way to crash
}


