#pragma once
#include "../../ext/http/httplib.h"
#include "../../ext/json/json.hpp"
#include <cstdint>
#include <string>

constexpr int PAYLOAD_SENT      = 200;
constexpr int PAYLOAD_ACK       = 201;
constexpr int PAYLOAD_MALFORMED = 400;

using json = nlohmann::json;

class Network
{
public:
	struct Data
	{
		std::string ipAddr;
		uint64_t    port = 0;
		json        body;
	};

	int SendData(Data d);
};