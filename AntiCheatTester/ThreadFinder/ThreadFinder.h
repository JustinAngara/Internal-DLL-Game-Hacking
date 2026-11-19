#pragma once
#include <Windows.h>
/*
IMPL comming soon
The idea of this is to figure out all loaded threads and see if any operations are done outside of the known threads
OR
To see if a arbritrary thread is 'spawned' and any operations is done (thus is a manual map detection)
*/
namespace ThreadFinder
{
	namespace Test
	{
		void T_CreateThread();                      // arbritrarily create thread
	}

	void RunEntry();
	void BaseThread();                           // arbritrarily reference bookkeep of thread
	bool BaseThreadInitThunk(uintptr_t nSource); //

}