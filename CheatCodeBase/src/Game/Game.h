#pragma once
#include <string>
#include "sdk/Logger/Logger.h"
#include "sdk/Memory/PatternScan.h"
#include <Windows.h>
#include <iostream>

class Game
{
public:
    Game(std::string name)
        : m_name(std::move(name)) { }
    virtual ~Game() = default;

    const std::string& getName() const { return m_name; }
    virtual void run() const = 0;
    

protected:
    std::string m_name;
    
};