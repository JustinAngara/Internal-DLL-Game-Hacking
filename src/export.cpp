#include "HackedGames/CounterStrike/Bhop.h"
#include "HackedGames/CounterStrike/CounterStrike.h"
#include "HackedGames/CounterStrike/Recoil.h"
#include "HackedGames/AssaultCube/AssaultCube.h"
#include "Obfuscation/HideProcess/HideProcess.h"
#include "Obfuscation/Registery/ObfuscateMethods.h"
#include "Game/GameManagement.h"


extern "C" __declspec(dllexport) void export_Bhop_run() {
    Bhop b;
    b.run();
}

extern "C" __declspec(dllexport) void export_Recoil_run() {
    Recoil r;
    r.run();
}

extern "C" __declspec(dllexport) void export_CounterStrike_run() {
    CounterStrike cs;
    cs.run();
}

extern "C" __declspec(dllexport) void export_AssaultCube_run() {
    AssaultCube ac;
    ac.run();
}

extern "C" __declspec(dllexport) void export_HideProcess_Run() {
    HideProcess::Run();
}

extern "C" __declspec(dllexport) void export_ObfuscationRegistry_Setup() {
    ObfuscationRegistry::Setup();
}

extern "C" __declspec(dllexport) void export_GameManagement_run() {
    auto game = std::make_shared<CounterStrike>();
    GameManagement gm(game);
    gm.runCurrentGame();
}