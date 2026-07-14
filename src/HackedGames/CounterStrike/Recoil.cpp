#include "Recoil.h"
#include "sdk/Math/Math.h"
#include <Windows.h>
// inteded for oen iteration
void Recoil::run() 
{
	
	static uintptr_t client  { (uintptr_t)GetModuleHandleA("client.dll") };
	static uintptr_t engine  { (uintptr_t)GetModuleHandleA("engine.dll") };
	static Math::Vec3 oPunch {0,0,0};
	uintptr_t lp{ *(uintptr_t*)(client + dwLocalPlayer) };
	Math::Vec3* angles{ (Math::Vec3*)(*(uintptr_t*)(engine + dwClientState) + dwClientState_viewAngles) };
	int* iShotsFired{ (int*)(lp)+m_iShotsFired };
	int* aimPunchAngle{ (int*)(lp)+m_aimPunchAngle };

	Math::Vec3 punch { *angles * 2 };
	Math::Vec3 newAngle{ *angles + oPunch - punch };
	newAngle.Normalize();
	*angles = newAngle;

	oPunch = punch;

}