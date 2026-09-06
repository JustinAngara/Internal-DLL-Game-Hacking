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
		std::string bios_info{};
		std::string proc_info{};
		std::string baseboard_info{};
		std::string mem_device{};
	};

	void SetInfo();
	Info GetInfo();
	

	
	// send to db
	int SendToServer(Info f);
}
