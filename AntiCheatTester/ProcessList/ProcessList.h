#pragma once
#include <Windows.h>
#include <vector>
#include <string>
namespace ProcList
{
	inline std::vector<std::string> procNames; 
	inline std::vector<std::string> forbiddenNames; // increase detections here
	void ListOutProcs();
	void PrintProcessNameAndID(DWORD processID);
	bool DoesProcExist(std::string procName);
}