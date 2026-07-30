#pragma once

// Game.h
class Game {
public:
    void Run();

    static Game* s_instance;
    static void RunTrampoline() { s_instance->Run(); }
private:
    int m_tickCount = 0;
    int m_age = 10;
    int m_score = 15;
};
