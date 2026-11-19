#include "ThreadFinder.h"
#include <DbgHelp.h>
#include <iostream>
void ThreadFinder::TF_CreateThread()
{
	static int Timer = 0;

	auto Result = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TestThread, &Timer, NULL, NULL);
	Timer++;
}


void ThreadFinder::TestThread() 
{
	std::cout << "This is a ent point to thread\n";
}

