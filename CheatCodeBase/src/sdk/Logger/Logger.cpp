#include "Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>


static std::ofstream logFile;

static std::string LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DBG:      return "DEBUG";
    case LogLevel::INFO:     return "INFO";
    case LogLevel::WARNING:  return "WARNING";
    case LogLevel::ERR:      return "ERROR";
    case LogLevel::CRITICAL: return "CRITICAL";
    default:                 return "UNKNOWN";
    }
}

void Logger::SetFileName(const std::string& filename)
{
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open())
        std::cerr << "Failed to open log file." << std::endl;
}

void Logger::Log(const std::string& message, LogLevel level)
{
    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    std::ostringstream entry;
    entry << "[" << timestamp << "] " << LevelToString(level) << ": " << message << "\n";
    
    if (logFile.is_open())
    {
        logFile << entry.str();
        logFile.flush();
    }
}

void Logger::Close()
{
    if (logFile.is_open())
        logFile.close();
}
