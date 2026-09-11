#pragma once

#define PAYLOAD_SENT 0x1000;

#define PAYLOAD_ACK 0x2000;

#define PAYLOAD_MALFORMED  0x9000;

// we are going to fix soon
// and we are going to think about what we want to do in terms of architecture

namespace Network
{
	class Payload
	{
		virtual void Send();
	};
	
	
}