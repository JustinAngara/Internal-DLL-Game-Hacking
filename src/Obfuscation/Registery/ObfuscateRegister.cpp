#include "ObfuscateRegister.h"
#include "../Polymorphism/Polymorphic.h"

static void ObfuscateRegister::AddFunc(std::string n, Vars::Func f)
{
	std::pair<std::string, Vars::Func> element{n,f};
	tableFuncs.push_back( element );
}


void ObfuscateRegister::Run()
{
	for (size_t i = 0; i < tableFuncs.size(); i++)
	{
				
	}
}

