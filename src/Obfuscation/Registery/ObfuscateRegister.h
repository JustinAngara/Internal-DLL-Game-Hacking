#pragma once

#include <vector>
#include <string>
#include "sdk/Utils/Structs.h"
#include "../Polymorphism/Polymorphic.h"
namespace ObfuscateRegister
{
    
    struct Entry {
        const char* name;
        Vars::Func        fn;    
    };
    // obfuscation techniques
    static CPolymorphic poly;


    // member variables and methods
    static std::vector< std::pair<std::string, Vars::Func> > tableFuncs;
	static void Run();
    static void AddFunc(std::string n, Vars::Func f);
	
}