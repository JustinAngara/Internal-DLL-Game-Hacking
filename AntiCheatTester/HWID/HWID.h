#pragma once
#include <string>


#define SAME_DEVICE_FOUND				 0x1000;
#define SAME_DEVICE_FOUND_FLAGGED_BANNED 0x1001;
#define SAME_DEVICE_FOUND_FLAGGED_ALT	 0x1002;

#define NEW_DEVICE_DETECTED 0x2000;


#define SERVER_RESPONSE_FAILED 0x4000;

namespace HWID
{
	struct Info
	{
		std::string biosInfo{};
		std::string procInfo{};
		std::string baseboardInfo{};
		std::string memDevice{};
	};

	void SetInfo();
	Info GetInfo();
	

	
	// send to db
	int SendToServer(Info f);


	uint64_t getHash(Info f);
	
	bool Compare(Info i1, Info i2); // this will be used for db stuff because we want to book keep how many times a hwid changes

}
