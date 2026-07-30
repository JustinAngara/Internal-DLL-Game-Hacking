#pragma once
#include "Game/Game.h"

class AssaultCube : public Game
{
public:
    AssaultCube() : Game("AssaultCube") { }
    ~AssaultCube() override = default;

    void run() const override;
};