#pragma once

#define PAYLOAD_SENT 0x1000;

#define PAYLOAD_ACK 0x2000;

#define PAYLOAD_MALFORMED  0x9000;

namespace Network
{
	int SendPayload();
	
}