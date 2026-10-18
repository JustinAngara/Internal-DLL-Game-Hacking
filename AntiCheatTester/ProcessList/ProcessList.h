#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <utility>
namespace Proc
{
	inline std::vector< std::pair<std::string, DWORD> > procList; 
	inline std::vector<std::string> forbiddenNames; // increase detections here
	void ListOutProcs();
	void PopulateProc(DWORD processID);
	bool DoesProcExist(std::string procName); // TODO
}