#pragma once

#include <array>
#include <cstddef> // Required for std::byte



#define PAYLOAD_SENT 0x200;

#define PAYLOAD_ACK 0x201;

#define PAYLOAD_MALFORMED  0x400;

// we are going to fix soon
// and we are going to think about what we want to do in terms of architecture
namespace Network
{
	struct Data {};
	class Payload
	{
	public:
		Data GetData();
	private:
	
		// switch to json later
		std::byte* byteArr;
		size_t size;

	};
	
	uint64_t SendPayload(Payload p);
	
}