#ifndef INPUT_H
#define INPUT_H

#include "../../CTR/Enums.h"
#include <3ds.h>
#include <string>
#include <vector>

class GamePad {
public:
    static Vectors::Vector2 GetLeftJoy();
    static Vectors::Vector2 GetRightJoy();
};

class Actions {
public:
    static bool Get(const std::string& keyName);
    static bool Down(const std::string& keyName);
    static bool Up(const std::string& keyName);
    static bool ActionDone(const std::string& keyName, u32 key);
};

class Mouse {
public:
    static Vectors::Vector2 position;
    static int screen;
    static bool screenRelativePosition;
    static bool multiTouch;
    static std::vector<Vectors::Vector2> multiTouchPositions;

    static void InitMouse()
    {
        screen = 1;
        screenRelativePosition = false;
        multiTouch = false;
    }

    static void UpdateMouse()
    {
        touchPosition touch;

		//Read the touch screen coordinates
		hidTouchRead(&touch);

        position = {touch.px, touch.py};
    }
};



#endif // INPUT_H
