#ifndef BUTTONS_H
#define BUTTONS_H

#include "Events.h"
#include "Input.h"

class Button {
public:
    Event OnClick;
    Event OnHoverEnter;
    Event OnHoverLeave;

    bool triggerOnRelease = false;
    bool enabled = true;
    Vector2 dimensions;
    Vector2 position;
    Vector2 Origin = {0.5f, 0.5f}; // Default origin center
    bool isHovering = false;

    Button(Vector2 dim, Vector2 pos)
        : dimensions(dim), position(pos) {}

    void UpdateButtonStatus()
    {
        if (!enabled) return;

        Vector2 mPos = Mouse::position;
        bool isPress = triggerOnRelease ? Actions::Up("Press") : Actions::Down("Press");

        Vector2 originOffset = Vector2(dimensions.x * Origin.x, dimensions.y * Origin.y);
        Vector2 topLeft = position - originOffset;
        Vector2 bottomRight = topLeft + dimensions;

        bool isIn = IsPointInsideBox(mPos, topLeft, bottomRight);
        bool wasHovering = isHovering;
        isHovering = isIn;

        if (wasHovering != isHovering) {
            if (isHovering)
                OnHoverEnter.Trigger();
            else
                OnHoverLeave.Trigger();
        }

        if (isHovering && isPress) {
            OnClick.Trigger();
        }
    }

private:
    bool IsPointInsideBox(const Vector2& point, const Vector2& topLeft, const Vector2& bottomRight)
    {
        return point.x >= topLeft.x && point.x <= bottomRight.x &&
               point.y >= topLeft.y && point.y <= bottomRight.y;
    }
};

#endif // BUTTONS_H