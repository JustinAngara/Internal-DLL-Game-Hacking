#pragma once

namespace ScreenShot
{

	bool TakeScreenshot(std::vector<BYTE>& pixels, LONG& outCx, LONG& outCy);
	bool SaveBmp(const wchar_t* path, const std::vector<BYTE>& pixels, LONG cx, LONG cy);

}
