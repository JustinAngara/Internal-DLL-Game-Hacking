#pragma once

#include <vector>
#include <utility>
#include <string>
#include "sdk/Utils/Structs.h"
#include "../Polymorphism/Polymorphic.h"
class ObfuscateRegister
{
public:
    struct Entry {
        const char* name;
        Vars::Func        fn;
    };
    // member variables and methods
    std::vector< std::pair<std::string, Vars::Func> > tableFuncs;

public:
    void Populate();
    void Run();
    void AddFunc(std::string n, Vars::Func f);
private:
};