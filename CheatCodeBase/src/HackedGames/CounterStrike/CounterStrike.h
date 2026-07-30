#pragma once
#include "Game/Game.h"

// this is just a sample of how to run games utilizing this. 
// TODO: remove hacked games into a own project and use ProcessHooker as the library to keep the two seperate.
class CounterStrike : public Game
{
public:
    CounterStrike() : Game("Counter Strike: Global Offensive") { }
    ~CounterStrike() override = default;

    void run() const override;
};