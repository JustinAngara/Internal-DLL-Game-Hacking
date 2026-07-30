#pragma once

#include <Windows.h>
#include "../Vars.h"



namespace AntiDbg
{
	enum AntiDbgMethod
	{
		TICK_COUNT,
		LOCAL_TIME,
		QPC
	};



	int CheckForTickCount(Func func);
	int CheckForLocalTime(Func func);
	int CheckForQPC(Func func);

	int RunHideThreadDebugger(LPTHREAD_START_ROUTINE ThreadMain);
	int CheckForDebugger(Func func, AntiDbgMethod method = TICK_COUNT);

}