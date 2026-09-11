#pragma once
#include <Windows.h>
#include <vector>
namespace ProcList
{
	std::vector<std::string> procNames; 
	std::vector<std::string> forbiddenNames; // increase detections here
	void ListOutProcs();
	void PrintProcessNameAndID(DWORD processID);
	bool DoesProcExist(std::string procName);
}