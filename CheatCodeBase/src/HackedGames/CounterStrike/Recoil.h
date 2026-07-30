#include <Windows.h>
class Recoil
{
public:
	void run();
private:
	const uintptr_t dwLocalPlayer{ 0x00 };
	const uintptr_t dwClientState{ 0x00};
	const uintptr_t dwClientState_viewAngles{ 0x00 };
	const uintptr_t m_iShotsFired{ 0x00 };
	const uintptr_t m_aimPunchAngle{ 0x00};

};