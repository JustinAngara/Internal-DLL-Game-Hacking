#pragma once
#include <string>
#include "../../Vars.h"


using TestFunc = void(*)();   

class Test
{
public:
	void Run(); // make sure to run isCalled, then m_func();
private:
	std::string m_name;
	bool        m_isCalled;
	TestFunc    m_func;
	bool        m_isSuccessful;
};

// enable tester
namespace Tester
{
	bool IsTestingOn    = true;

	bool IsGUIDisplayOn = true;
	bool EnableLogging  = true;

	
}