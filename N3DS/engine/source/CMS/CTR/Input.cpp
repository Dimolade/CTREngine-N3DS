#include "Input.h" // Include the correct header
#include "../../CTR/Enums.h"
#include <3ds.h>

Vectors::Vector2 Mouse::position;
int Mouse::screen;
bool Mouse::screenRelativePosition;
bool Mouse::multiTouch;
std::vector<Vectors::Vector2> Mouse::multiTouchPositions;

Vectors::Vector2 GamePad::GetLeftJoy()
{
    circlePosition pos;
    hidCircleRead(&pos);
    return {pos.dx / 156.0f, pos.dy / 156.0f};
}

Vectors::Vector2 GamePad::GetRightJoy()
{
    circlePosition pos;
    hidCstickRead(&pos);
    return {pos.dx / 156.0f, pos.dy / 156.0f};
}

bool Actions::Get(const std::string& keyName)
{
    return ActionDone(keyName, hidKeysHeld());
}

bool Actions::Down(const std::string& keyName)
{
    return ActionDone(keyName, hidKeysDown());
}

bool Actions::Up(const std::string& keyName)
{
    return ActionDone(keyName, hidKeysUp());
}

bool Actions::ActionDone(const std::string& keyName, u32 key)
{
    if (keyName == "Menu")
    {
        return key & KEY_START;
    }
    if (keyName == "Select")
    {
        return key & KEY_TOUCH;
    }
    if (keyName == "FireL")
    {
        return key & KEY_L;
    }
    if (keyName == "FireR")
    {
        return key & KEY_R;
    }
    if (keyName == "FireL2")
    {
        return key & KEY_ZL;
    }
    if (keyName == "FireR2")
    {
        return key & KEY_ZR;
    }
    if (keyName == "Press")
    {
        return key & KEY_TOUCH;
    }
    if (keyName == "Left")
    {
        return key & KEY_DLEFT;
    }
    if (keyName == "Right")
    {
        return key & KEY_DRIGHT;
    }
    if (keyName == "Up")
    {
        return key & KEY_DUP;
    }
    if (keyName == "Down")
    {
        return key & KEY_DDOWN;
    }
    if (keyName == "MoveLeft")
    {
        return key & KEY_CPAD_LEFT;
    }
    if (keyName == "MoveRight")
    {
        return key & KEY_CPAD_RIGHT;
    }
    if (keyName == "MoveUp")
    {
        return key & KEY_CPAD_UP;
    }
    if (keyName == "MoveDown")
    {
        return key & KEY_CPAD_DOWN;
    }
    if (keyName == "A1")
    {
        return key & KEY_A;
    }
    if (keyName == "A2")
    {
        return key & KEY_B;
    }
    if (keyName == "A3")
    {
        return key & KEY_X;
    }
    if (keyName == "A4")
    {
        return key & KEY_Y;
    }
    return false;
}
