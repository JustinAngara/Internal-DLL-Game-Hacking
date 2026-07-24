#include "ObfuscateRegister.h"
#include "../Polymorphism/Polymorphic.h"

static void ObfuscateRegister::AddFunc(std::string n, Func f)
{
	std::pair<std::string, Func> element{n,f};
	tableFuncs.push_back( element );
}


void ObfuscateRegister::Run()
{
	for (size_t i = 0; i < tableFuncs.size(); i++)
	{
				
	}
}

