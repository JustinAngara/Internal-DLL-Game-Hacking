#include "sdk/Logger/Logger.h"
#include "Main.h"
#include "Game/AssaultCube/AssaultCube.h"
#include "Game/Game.h"
#include "Game/GameManagement.h"
#include <Windows.h>
void Main::entrypoint()
{

    // create console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    // assault cube
    auto assaultCube = std::make_shared<AssaultCube>();
    GameManagement gm(assaultCube);
    gm.addGame(assaultCube);
    gm.runCurrentGame(); 
    

    // close console
    fclose(f);
    FreeConsole();
}

