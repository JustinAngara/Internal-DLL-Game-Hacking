#pragma once

#include "../Network/Network.h"

class ServerToClient : public Network
{
	httplib::Server server;
	std::mutex      dataMutex;
	void Listen();
};