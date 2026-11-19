#include "ThreadFinder.h"

#include <DbgHelp.h>
#include <iostream>
void ThreadFinder::Test::T_CreateThread()
{
	static int Timer = 0;

	auto Result = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)BaseThread, &Timer, NULL, NULL);
	Timer++;
}


void ThreadFinder::BaseThread() 
{

	std::cout << "This is a ent point to thread\n";
}



void ThreadFinder::RunEntry()
{
	while (true)
	{
		if (GetAsyncKeyState(0x46))
		{
			std::cout << "Key pressed for Thread Runner\n";
			Sleep(10);
		}
	}
}

