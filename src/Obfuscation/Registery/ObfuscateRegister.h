#pragma once

#include <vector>
#include <string>
#include "../Polymorphism/Polymorphic.h"
namespace ObfuscateRegister
{
    using Func = void(*)();       

    struct Entry {
        const char* name;
        Func        fn;    
    };
    // obfuscation techniques
    static CPolymorphic poly;


    // member variables and methods
    static std::vector< std::pair<std::string, Func> > tableFuncs;
	static void Run();
    static void AddFunc(std::string n, Func f);
	
}