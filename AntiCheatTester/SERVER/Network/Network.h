#pragma once

#include <array>
#include <cstddef> // Required for std::byte
#include "../../ext/json/json.hpp"
#include "../../ext/http/httplib.h"

#define PAYLOAD_SENT 0x200;

#define PAYLOAD_ACK 0x201;

#define PAYLOAD_MALFORMED  0x400;

using json = nlohmann::json;

	
// we are going to fix soon
// and we are going to think about what we want to do in terms of architecture
class Network
{
public:
	struct Data
	{
		std::string ipAddr;
		uint64_t    port;
		json body;
	};

	int SendData(Data d);
};