#pragma once
#include <string>
#include <unordered_map>
#include <memory>


enum class SEVERITY 
{
    LOW,
    NORMAL,
    HIGH
};

class LogBucket 
{
public:
    LogBucket(std::string name) 
             : m_name(name) {}

    bool isReadyToPublish() const { return m_isReadyToPublish; }
    void setReadyToPublish(bool a)      { m_isReadyToPublish = a; }

private:
    SEVERITY    m_severity         = SEVERITY::NORMAL;
    std::string m_name             = "";
    std::string m_description      = "";
    bool        m_isReadyToPublish = false; 
};

namespace Logger 
{
    inline std::string g_fileLoc = "";
    inline std::unordered_map<std::string, std::unique_ptr<LogBucket>> g_logBuckets{};
    
    void RegisterBucket(const std::string& name);
    LogBucket* GetInstanceOfBucket(const std::string& name); 
}