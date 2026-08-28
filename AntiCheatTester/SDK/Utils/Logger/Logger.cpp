#include "Logger.h" 

LogBucket* Logger::GetInstanceOfBucket(const std::string& name) 
{
    auto it = g_logBuckets.find(name);

    if (it != g_logBuckets.end()) 
    {
        return it->second.get();
    }

    return nullptr; 
}

void Logger::RegisterBucket(const std::string& name) 
{
    if (g_logBuckets.find(name) == g_logBuckets.end()) 
    {
        g_logBuckets[name] = std::make_unique<LogBucket>(name);
    }
}