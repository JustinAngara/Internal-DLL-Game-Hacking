#include "Tests.h"
#include "../Network/Network.h"
#include "../ClientToServer/Client.h"
#include "../ServerToClient/Server.h"
void Tests::ToListen()
{
    ServerToClient s;
    s.Listen();
}