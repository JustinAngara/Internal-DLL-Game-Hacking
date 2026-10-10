
#pragma once
#include "../../ext/http/httplib.h"
#include "../../ext/json/json.hpp"
#include <mutex>
#include "../Network/Network.h"

class ServerToClient : public Network
{
public:
	void Listen();
private:
	httplib::Server server;
	std::mutex      dataMutex;
};