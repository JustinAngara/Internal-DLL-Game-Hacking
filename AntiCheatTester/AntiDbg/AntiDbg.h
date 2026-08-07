#pragma once

#include <Windows.h>
#include "../Vars.h"



namespace AntiDbg
{
	const DWORD64 MAX_ALLOWED_CYCLES = 50000000;
	enum AntiDbgMethod
	{
		TICK_COUNT,
		LOCAL_TIME,
		QPC,
		RDTSC
	};



	DWORD64 CheckForTickCount(Func func);
	DWORD64 CheckForLocalTime(Func func);
	DWORD64 CheckForQPC(Func func);
	DWORD64 CheckForRDTSC(Func func);

	DWORD64 CheckForDebugger(Func func, AntiDbgMethod method = TICK_COUNT);
	HANDLE RunHideThreadDebugger(LPTHREAD_START_ROUTINE ThreadMain);
	HANDLE RunThreadEx(LPTHREAD_START_ROUTINE ThreadEx);

	namespace ChildProc
	{
		void EnsureDebuggingOccurs();
		void CreateChildProc(char* argv[]);
		void ChildGuard(int argc, char* argv[]);
		void ValidateAliveChild();
	}
}