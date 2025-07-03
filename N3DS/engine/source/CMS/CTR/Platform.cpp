#include "Platform.h"

int Platform::PrimaryScreen = 0;
int Platform::TouchScreen = 1;
int Platform::ScreenCount = 2;
int Platform::AllScreenIndex = 2;
std::string Platform::name = "Nintendo 3DS";

ScreenInfo Platform::Screens[2] = {
    ScreenInfo(400, 240, true, false, "Upper Screen"),
    ScreenInfo(320, 240, false, true, "Lower Screen")
};

ScreenInfo Platform::GetScreen(int si)
{
	return Platform::Screens[si];
}
