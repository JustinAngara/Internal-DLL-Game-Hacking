#include "Helper.h"

std::wstring Helper::str2wstr(const std::string& str)
{
    if (str.empty()) return {};

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (size <= 0) throw std::runtime_error("MultiByteToWideChar failed");

    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], size);
    return result;
}


std::string Helper::wstr2str(const std::wstring& wstr)
{
    if (wstr.empty()) return {};

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    //if (size <= 0) throw std::runtime_error("WideCharToMultiByte failed");

    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
} 