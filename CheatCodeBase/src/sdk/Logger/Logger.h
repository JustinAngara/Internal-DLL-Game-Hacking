#pragma once
#include <string>

enum class LogLevel { DBG, INFO, WARNING, ERR, CRITICAL };

namespace Logger 
{
    void SetFileName(const std::string& filename);
    void Log(const std::string& message, LogLevel level = LogLevel::INFO);
    void Close();
}