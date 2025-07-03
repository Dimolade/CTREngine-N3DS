#ifndef BLOBBYFONT_H
#define BLOBBYFONT_H

#include <vector>
#include <3ds.h>
#include <citro2d.h>
#include <cmath> // For sin, cos, and M_PI
#include "CTR/Enums.h"

using namespace std;
using namespace Vectors;

struct BlobbyFontChar
{
    char c;
    string address;
    int addressInd;
    int width;
    int height;
    C2D_SpriteSheet spriteSheet;

    void Init()
    {
        spriteSheet = C2D_SpriteSheetLoad(address.c_str());
        /*C2D_SpriteFromSheet(&spr, spriteSheet, index);
        C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
        C2D_SpriteSetPos(&spr, pos.x, pos.y);
        C2D_SpriteSetRotationDegrees(&spr,rotation.z);
        C2D_SpriteSetScale(&spr, size.x,size.y);*/
    }

    BlobbyFontChar()
        : c('\0'), address(""), addressInd(0), width(0), height(0), spriteSheet(nullptr)
    {}

    BlobbyFontChar(char _c, const std::string& _address, int _addressInd, int _width, int _height)
        : c(_c), address(_address), addressInd(_addressInd), width(_width), height(_height)
    {Init();}
};

class BlobbyFont
{
public:
    std::string name;
    std::vector<BlobbyFontChar> Chars;
    BlobbyFontChar defaultChar;
    std::string id;

    // Constructor
    BlobbyFont(const std::string& fontName, const std::vector<BlobbyFontChar>& chars, const BlobbyFontChar& defChar, std::string iid = "nothing")
        : name(fontName), Chars(chars), defaultChar(defChar), id(iid)
    {
        defaultChar.Init();
        for (BlobbyFontChar c : Chars)
        {
            c.Init();
        }
    }

    void AddChar(BlobbyFontChar car)
    {
        if (!car.spriteSheet) {
            car.Init(); // Only load once
        }
        Chars.push_back(car);
    }
};

struct BlobbyChar
{
    BlobbyFontChar BFC;
    float x;
    float y;
    float sx;
    float sy;
    float rz;
    bool visible = true;
    Color color;
    C2D_Sprite spr;

    BlobbyChar()
        : x(0), y(0), sx(1), sy(1), rz(0), visible(true), color({255, 255, 255, 255, 0})
    {}

    void Init()
    {
        C2D_SpriteFromSheet(&spr, BFC.spriteSheet, BFC.addressInd);
        color = {255,255,255,255,0};
        visible = true;
    }

    void UpdateTransform()
    {
        C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
        C2D_SpriteSetPos(&spr, x, y);
        C2D_SpriteSetRotationDegrees(&spr,rz);
        C2D_SpriteSetScale(&spr, sx,sy);
    }

    void Render()
    {
        if (visible) {
            C2D_ImageTint tint = GetColor();
            C2D_DrawSpriteTinted(&spr, &tint); // Pass as pointer
        }
    }

    C2D_ImageTint GetColor()
    {
        C2D_ImageTint tint;

        C2D_Tint colorTint = {C2D_Color32(color.r, color.g, color.b, color.a), color.blend/255};

        // Apply the same tint to all four corners
        tint.corners[0] = colorTint;
        tint.corners[1] = colorTint;
        tint.corners[2] = colorTint;
        tint.corners[3] = colorTint;

        return tint;
    }
};

class BlobbyText
{
public:
    std::string text;
    BlobbyFont* font;
    std::vector<BlobbyChar*> Characters;
    float x;  // Base X position
    float y;  // Base Y position
    float sx; // Base scale X
    float sy; // Base scale Y
    float rz; // Base rotation Z (in radians)
    Color col;

    // Constructor
    BlobbyText(const std::string& t, BlobbyFont* ifont, float ix, float iy, float isx, float isy, float irz, Color ic) :
    text(t), font(ifont), x(ix), y(iy), sx(isx), sy(isy), rz(irz), col(ic)
    {
        SetText(text);  // Initialize the text upon creation
    }

    BlobbyText()
        : text(""),
        font(nullptr),
        x(0), y(0),
        sx(1), sy(1),
        rz(0)
    {}

    int GetStringX(const std::string& Te)
    {
        int cc = 0;
        for (char c : Te) {
            if (c == '\n') {
                // nothin
            } else {
                cc += GetCharacter(c).width;
            }
        }
        return cc;
    }

    // Set text and initialize characters
    void SetText(const std::string& Te, float lineHeight = 12)
    {
        Characters.clear();  // Clear previous characters
        Characters.reserve(Te.length());
        text = Te;
        float currentX = x;  // Start at the base X position
        float currentY = y;  // Start at the base Y position

        // Convert the rotation from degrees to radians
        float rzInRadians = rz * (M_PI / 180.0f);

        for (char c : Te) {
            if (c == '\n') {
                // When newline is encountered, move to the next line
                currentX = x; // Reset X to the starting position
                currentY += lineHeight; // Move Y down by line height
            } else {
                BlobbyFontChar cf = GetCharacter(c);

                // Dynamically allocate memory for BlobbyChar
                BlobbyChar* blobbyChar = new BlobbyChar();  // Ensure new memory is allocated
                blobbyChar->BFC = cf;

                // Apply scaling to the character
                blobbyChar->sx = sx;
                blobbyChar->sy = sy;

                // Apply rotation (around Z-axis) to the character
                float cosRz = cos(rzInRadians);
                float sinRz = sin(rzInRadians);

                // Original position of the character (use currentX and currentY)
                float originalX = currentX;
                float originalY = currentY;

                // Apply rotation to the position (rotate around the origin)
                float rotatedX = originalX * cosRz - originalY * sinRz;
                float rotatedY = originalX * sinRz + originalY * cosRz;

                // Apply scale to the rotated position
                blobbyChar->x = rotatedX * blobbyChar->sx;
                blobbyChar->y = rotatedY * blobbyChar->sy;

                // Set the rotation (rz) for the character as well
                blobbyChar->rz = rz;  // Store the rotation value in the BlobbyChar object

                blobbyChar->Init();  // Initialize character
                blobbyChar->UpdateTransform();  // Update transformations

                // Store this character
                blobbyChar->color = col;
                blobbyChar->visible = true;
                Characters.push_back(blobbyChar);

                // Update currentX for the next character position
                currentX += cf.width * sx;  // You can also include spacing or kerning here
            }
        }
    }

    // Get the BlobbyFontChar for a given character
    BlobbyFontChar GetCharacter(char c)
    {
        for (BlobbyFontChar fc : font->Chars)
        {
            if (fc.c == c)
            {
                return fc;
            }
        }
        return font->defaultChar;  // Return a default BlobbyFontChar if not found
    }

    // Render all the characters
    void Render()
    {
        for (BlobbyChar* bc : Characters)
        {
            bc->Render();
        }
    }
};

#endif