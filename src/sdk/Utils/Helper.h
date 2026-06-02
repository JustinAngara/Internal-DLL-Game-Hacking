#pragma once

#include <Windows.h>
#include <string>
#include <stdexcept>

namespace Helper
{
    std::wstring str2wstr(const std::string& str);
    std::string wstr2str(const std::wstring& wstr);
}