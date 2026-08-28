#pragma once
#include <vector>
#include <string>
// lets use the logger this time rather than random print statements
/*

one central logger
-> buckets
	-> name
	-> severity
	-> message
*/

enum SEVERITY
{
	LOW,
	NORMAL,
	HIGH
};

class LogBucket
{
public:
	bool isReadyToPublish()  { return m_isReadyToPublish; }
	void setReadyToPublish() { m_isReadyToPublish = true; }
	
private:
	SEVERITY    m_severity;
	std::string m_name;
	std::string m_description;
	bool        m_isReadyToPublish;

};

namespace Logger
{
	// save to file
	std::string g_fileLoc = "";
	std::vector<LogBucket> g_logBucket{};
	LogBucket* GetInstanceOfBucket(std::string name); // searchup for a name of a bucket, and then return a reference of it

}



