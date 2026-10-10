
#pragma once
#include "../../ext/http/httplib.h"
#include "../../ext/json/json.hpp"
#include <mutex>
#include "../Network/Network.h"

class ServerToClient : public Network
{
	httplib::Server server;
	std::mutex      dataMutex;
	void Listen();
};