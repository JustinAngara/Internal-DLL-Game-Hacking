#include "ObfuscateRegister.h"
#include "../Polymorphism/Polymorphic.h"
#include "../SpoofReturnAddress/SpoofRet.h"

void Foo()
{
	int buf[2000];
	char buff[2000];
	bool lalala{1};

	memset(buf,  1337, sizeof(buf));
	memset(buff, 48, sizeof(buff));
	
	
}

void ObfuscateRegister::Populate()
{
	Vars::Func f = Foo;
	AddFunc("test_foo", f);
}

void ObfuscateRegister::AddFunc(std::string n, Vars::Func f)
{
	std::pair<std::string, Vars::Func> element{n,f};
	tableFuncs.push_back( element );
}

// static std::vector< std::pair<std::string, Vars::Func> > tableFuncs;
	
void ObfuscateRegister::Run()
{
	SpoofRet s; 
	CPolymorphic c;
	for (auto& e : tableFuncs)
	{
		// probably want to call spoof ret address
		// polymorphic engine
		uintptr_t funcAddr = reinterpret_cast<uintptr_t>(e.second);
		c.Run(funcAddr);
		s.Run(e.second);

	}
}

