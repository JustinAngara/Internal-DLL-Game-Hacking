#pragma once
#include "Game/Game.h"

class CounterStrike : public Game
{
public:
    CounterStrike() : Game("Counter Strike: Global Offensive") { }
    ~CounterStrike() override = default;

    void run() const override;
};