#include <Windows.h>
#include <vector>




bool SaveBmp(const wchar_t* path, const std::vector<BYTE>& pixels, LONG cx, LONG cy)
{
    BITMAPINFOHEADER bih{};
    bih.biSize     = sizeof(bih);
    bih.biWidth    = cx;
    bih.biHeight   = -cy;
    bih.biPlanes   = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = static_cast<DWORD>(pixels.size());

    BITMAPFILEHEADER bfh{};
    bfh.bfType    = 0x4D42;  // "BM"
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
    bfh.bfSize    = bfh.bfOffBits + bih.biSizeImage;

    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    WriteFile(h, &bfh, sizeof(bfh), &written, nullptr);
    WriteFile(h, &bih, sizeof(bih), &written, nullptr);
    WriteFile(h, pixels.data(), static_cast<DWORD>(pixels.size()), &written, nullptr);
    CloseHandle(h);
    return true;
}

bool TakeScreenshot(std::vector<BYTE>& pixels, LONG& outCx, LONG& outCy)
{
    HWND hwnd = FindWindowW(nullptr, L"title add implementation soon");
    if (!hwnd) return false;

    RECT rc{};
    GetClientRect(hwnd, &rc);
    const LONG cx = rc.right - rc.left;
    const LONG cy = rc.bottom - rc.top;
    if (cx <= 0 || cy <= 0) return false;

    HDC hdcSrc = GetDC(hwnd);
    if (!hdcSrc) return false;

    HDC     hdcDst = CreateCompatibleDC(hdcSrc);
    HBITMAP hbmp   = CreateCompatibleBitmap(hdcSrc, cx, cy);
    HGDIOBJ oldObj = SelectObject(hdcDst, hbmp);

    BOOL blitOk = BitBlt(hdcDst, 0, 0, cx, cy, hdcSrc, 0, 0, SRCCOPY);

    SelectObject(hdcDst, oldObj);          

    BITMAPINFO info{};
    info.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth       = cx;
    info.bmiHeader.biHeight      = -cy;    
    info.bmiHeader.biPlanes      = 1;
    info.bmiHeader.biBitCount    = 32;     // BGRA
    info.bmiHeader.biCompression = BI_RGB;

    // save data into buffer
    pixels.resize(static_cast<size_t>(cx) * cy * 4);
    int lines = GetDIBits(hdcDst, hbmp, 0, cy, pixels.data(), &info, DIB_RGB_COLORS);

    // cleanup
    DeleteObject(hbmp);
    DeleteDC(hdcDst);
    ReleaseDC(hwnd, hdcSrc);

    outCx = cx;
    outCy = cy;
    return blitOk && lines == cy;
}

