#ifndef ENUMS_H
#define ENUMS_H

namespace Vectors
{

struct Vector2 {
    float x, y;

    // Overload * operator to scale Vector2 by a scalar
    Vector2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    Vector2 operator+(Vector2 add) const {
        return {x+add.x, y+add.y};
    }

    Vector2 operator-(Vector2 add) const {
        return {x-add.x, y-add.y};
    }

    Vector2() : x(0.0f), y(0.0f) {}

    Vector2(float x1, float y1) : x(x1), y(y1) {}
};

struct Vector3 {
    float x, y, z;

    // Overload += operator to add a Vector2 to Vector3
    Vector3& operator+=(const Vector2& other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    // Overload * operator to scale Vector3 by a scalar
    Vector3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    Vector3(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {}
    Vector3() : x(0.0f), y(0.0f), z (0.0f) {}
};

struct Color {
    int r;
    int g;
    int b;
    int a;
    float blend;

    Color(int r2, int g2, int b2, int a2, float bl2) : r(r2), g(g2), b(b2), a(a2), blend(bl2) {}
    Color() : r(255), g(255), b(255), a(255), blend(255) {}
};
}

class Enums
{
public:
    enum class AssetType { // Cambiado a enum class
        Image,
        Script,
        Font,
        Sound,
        Other,
        ImageFont
    };

    enum class AssetSortType { // Cambiado a enum class
        Name,
        AssetType,
        Both
    };
};

#endif // ENUMS_H
