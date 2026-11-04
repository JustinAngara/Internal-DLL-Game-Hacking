#pragma once
#include "../../ext/http/httplib.h"
#include "../../ext/json/json.hpp"
#include <mutex>
#include "../Network/Network.h"

class ClientToServer : public Network
{
public:
	void Send();
private:
	httplib::Server server;
	std::mutex      dataMutex;

};