#ifndef CLASSES_H
#define CLASSES_H

#include <citro2d.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <3ds.h>
#include "Enums.h"
#include "../BlobbyFont.h"
#include "../BlobbyAudio.h"
#include "../CMS/CTR/Debug.hpp"
#include <vector>
#include <algorithm>
#include <memory>
#include "../CMS/CTR/lodepng.h"

using namespace std;
using namespace Vectors;

class CTREntry;

extern vector<CTREntry*> entryPoints;

class CTREntry
{
public:

    virtual void EntryPoint() {};
    virtual void OnFrame() {};
    virtual void AfterFrame() {};
    CTREntry() {
        // Optional: register instance on construction
        entryPoints.push_back(this);
    }

    virtual ~CTREntry() {}
};

class CTRBlobbyFonts {
public:
    static vector<BlobbyFont*> fonts;
    static void InitFonts();
};

class GameAsset;

extern vector<GameAsset*> GameAssets;

class GameAsset {
public:
    Enums::AssetType type;
    bool InstantLoad;
    string name;
    string Namespace;
    string assetPath;
    int screenIndex;
    GameAsset(Enums::AssetType t, string ap, bool il = false, string n = "default", string nspace = "default", int sI = 0) : type(t), InstantLoad(il), name(n), Namespace(nspace), assetPath(ap), screenIndex(sI) {} // Constructor initializes type

    virtual ~GameAsset() {}
};

class CTRAxyz : public GameAsset
{
public:
    Vector3 position;
    Vector3 size;
    Vector3 rotation;
    CTRAxyz* Link;
public:
    Vector3 renderPosition;
    Vector3 renderSize;
    Vector3 renderRotation;

    void UpdateLink()
    {
        if (Link == nullptr)
        {
            renderPosition = position;
            renderRotation = rotation;
            renderSize = size;
            return;
        }

        Vector3 parentRot = Link->renderRotation;

        Vector3 rotatedPos = RotatePoint(position, parentRot);

        renderPosition = Link->renderPosition + rotatedPos;

        renderRotation = Link->renderRotation + rotation;

        renderSize = Link->renderSize * size;
    }

    Vector3 RotatePoint(Vector3 point, Vector3 rotDegrees)
    {
        Vector3 result = point;

        float rx = rotDegrees.x * (M_PI / 180.0f);
        float ry = rotDegrees.y * (M_PI / 180.0f);
        float rz = rotDegrees.z * (M_PI / 180.0f);

        float x1 = result.x * cos(rz) - result.y * sin(rz);
        float y1 = result.x * sin(rz) + result.y * cos(rz);
        result.x = x1; result.y = y1;

        float z1 = result.z * cos(ry) - result.x * sin(ry);
        float x2 = result.z * sin(ry) + result.x * cos(ry);
        result.z = z1; result.x = x2;

        float y2 = result.y * cos(rx) - result.z * sin(rx);
        float z2 = result.y * sin(rx) + result.z * cos(rx);
        result.y = y2; result.z = z2;

        return result;
    }
public:

    //CTRAxyz(Vector3 p, Vector3 s, Vector3 r) : position(p), size(s), rotation(r) {}
    CTRAxyz(Vector3 p, Vector3 s, Vector3 r, Enums::AssetType t, string ap, bool il = false, string n = "default", string nspace = "default", int sI = 0)
    : position(p), size(s), rotation(r), GameAsset(t,ap,il,n,nspace,sI) {}
};

class CTRCamera;

extern vector<CTRCamera*> CAMERAS;

struct CTRScissor
{
public:
    Enums::CTRScissorMode mode;
    Vector2 position;
    Vector2 dimensions;

    void Set()
    {
        set_scissor_simple((GPU_SCISSORMODE)mode, position.x, position.y, dimensions.x, dimensions.y);
    }

    void SetNone()
    {
        set_scissor_simple(GPU_SCISSORMODE::GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
    }

    CTRScissor(Enums::CTRScissorMode m, Vector2 pos, Vector2 dim) : mode(m), position(pos), dimensions(dim) {}
    CTRScissor(int m, Vector2 pos, Vector2 dim) : mode(static_cast<Enums::CTRScissorMode>(m)), position(pos), dimensions(dim) {}

private:
    void set_scissor(GPU_SCISSORMODE mode, int x, int y, int width, int height) {
        int inv_y = 400 - (y + height);
        int inv_x = 240 - (x + width);

        C2D_Flush();

        C3D_SetScissor(mode,
            inv_y,            // left (Y min)
            inv_x,                // top (X min)
            inv_y + height,    // right (Y max)
            inv_x + width    // bottom (X max)
        );
	}
		
	void set_scissor_simple(GPU_SCISSORMODE mode, int x, int y, int width, int height) {
        C2D_Flush();

        C3D_SetScissor(mode,
            y+height,            // left (Y min) // 0
            x+width,                // top (X min) // 0
            240-y,    // right (Y max) //0
            400-x    // bottom (X max) // 0
        );
    }
};

class CTRGraphic : public CTRAxyz
{
public:
    Color color;
    bool visible;
    bool loadOnShow;

    CTRGraphic(Color c, bool vi, bool los ,Vector3 p, Vector3 s, Vector3 r, Enums::AssetType t, string ap, bool il = false, string n = "default", string nspace = "default", int sI = 0) :
    color(c), visible(vi), loadOnShow(los), CTRAxyz(p,s,r, t,ap,il,n,nspace,sI) {}
};

class CTRSound : public GameAsset {
public:
    bool playOnLoad;
    BlobbyAudio blobbyAudio;

    void SetClip(string ap = "")
    {
        if (ap == "")
        {
            ap = assetPath;
        }
        else
        {
            assetPath = ap;
        }
        if (assetPath.rfind("romfs:/", 0) != 0)
        {
            assetPath = ("romfs:/snd/"+assetPath+".bin");
        }
        /*Log::Append("Trying to load Sound: "+assetPath+"\n");
        Log::Save();*/
        blobbyAudio.LoadClip(assetPath);
    }

    /*void SetClipPCM(const int16_t* pcmData, size_t sampleCount, int sampleRate = 22050, bool stereo = false)
    {
        blobbyAudio.LoadPCM(pcmData, sampleCount, sampleRate, stereo);
    }

    void SetClipPCM(const std::vector<int16_t>& pcmData, int sampleRate = 22050, bool stereo = false)
    {
        blobbyAudio.LoadPCM(pcmData, sampleRate, stereo);
    }*/

    void Play()
    {
        blobbyAudio.Play();
    }

    void Stop()
    {
        blobbyAudio.Stop();
    }

    void Pause()
    {
        blobbyAudio.Pause();
    }

    void SetStereo(float stereo)
    {
        blobbyAudio.SetPan(stereo);
    }

    void SetSpeed(float speed)
    {
        blobbyAudio.SetSpeed(speed);
    }

    void SetVolume(float volume)
    {
        blobbyAudio.SetVolume(volume);
    }

    void SetLooping(bool l)
    {
        blobbyAudio.SetLooping(l);
    }

    float GetStereo()
    {
        return blobbyAudio.GetPan();
    }

    float GetSpeed()
    {
        return blobbyAudio.GetSpeed();
    }

    float GetVolume()
    {
        return blobbyAudio.GetVolume();
    }

    bool IsLooping()
    {
        return blobbyAudio.IsLooping();
    }

    bool IsPlaying()
    {
        return blobbyAudio.IsPlaying();
    }

    CTRSound(string n, string nspace, string ap, int si, bool iL, bool pOL) :
    GameAsset(Enums::AssetType::Sound, ap, iL, n,nspace, si),
    playOnLoad(pOL)
    {
        SetClip(ap);
    }
};

class CTRImageFont : public CTRGraphic {
public:
    std::string Text;
    BlobbyText* textH;

    ~CTRImageFont() {
        delete textH;
    }

    void render() {
        if (textH != nullptr)
        textH->Render();
    }

    void UpdateProperties()
    {
        if (textH == nullptr) return;
        textH->x = position.x;
        textH->y = position.y;
        textH->sx = size.x;
        textH->sy = size.y;
        textH->rz = rotation.z;
        textH->col = color;
        textH->SetText(Text);
        UpdateLink();
    }

    int GetLengthInPX()
    {
        if (textH != nullptr)
        return textH->GetStringX(Text);
        else return -1;
    }

    CTRImageFont(
        string ap,
        bool v,
        bool los,
        Vector3 p,
        Vector3 s,
        Vector3 rot,
        string n,
        string nspace,
        bool instaLoad,
        Color c,
        int screenIndex,
        std::string t
    )
    : CTRGraphic(c, v, los, p, s, rot,Enums::AssetType::ImageFont, ap, instaLoad, n,nspace, screenIndex),
    Text(t)
    {}
};

struct TrackedSpriteSheet {
    std::string path;
    C2D_SpriteSheet sheet = nullptr;
    int users = 0;

    TrackedSpriteSheet(const std::string& p) : path(p) {
        sheet = C2D_SpriteSheetLoad(p.c_str());
    }

    ~TrackedSpriteSheet() {
        if (sheet) {
            C2D_SpriteSheetFree(sheet);
        }
    }

    static TrackedSpriteSheet* GetOrCreate(const std::string& assetPath) {
        for (auto& s : LoadedSS) {
            if (s && s->path == assetPath) {
                s->users++;
                return s;
            }
        }

        TrackedSpriteSheet* newSheet = new TrackedSpriteSheet(assetPath);
        if (!newSheet->sheet) {
            delete newSheet;
            return nullptr;
        }

        newSheet->users = 1;
        LoadedSS.push_back(newSheet);
        return newSheet;
    }

    static void Cleanup() {
        for (auto it = LoadedSS.begin(); it != LoadedSS.end(); ) {
            if (*it && (*it)->users <= 0) {
                delete *it;
                it = LoadedSS.erase(it);
            } else {
                ++it;
            }
        }
    }

    static std::vector<TrackedSpriteSheet*> LoadedSS;
};

class C2DImageLoaderC2D
{
public:
    static bool loadPngImage(C2D_Image* image, string path) {
        //char path[128] = "sdmc:/22.png";
        //snprintf(path, sizeof(path), "/3ds/switch/icons/%016llX.png", titleId);

        unsigned char* pngData = NULL;
        unsigned width = 0, height = 0;
		const unsigned char* d = reinterpret_cast<const unsigned char*>(path.data()); 
        unsigned error = lodepng_decode32(&pngData, &width, &height, d, path.size());

        if (error || pngData == NULL) {
            return false;
        }

        if (width != 256 || height != 256) {
            free(pngData);
            return false;
        }

        C3D_Tex* tex = (C3D_Tex*)linearAlloc(sizeof(C3D_Tex));
        Tex3DS_SubTexture* subtex = (Tex3DS_SubTexture*)linearAlloc(sizeof(Tex3DS_SubTexture));
        if (!tex || !subtex) {
            if (tex) linearFree(tex);
            if (subtex) linearFree(subtex);
            free(pngData);
            return false;
        }


        C3D_TexInit(tex, 256, 256, GPU_RGBA8);
        C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);
        tex->border = 0xFFFFFFFF;

        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                u32 dstPos = ((((y >> 3) * (256 >> 3) + (x >> 3)) << 6) +
                            ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) |
                            ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * 4;
                u32 srcPos = (y * width + x) * 4;
                u8* dst = (u8*)tex->data + dstPos;

                dst[0] = pngData[srcPos + 3]; // Alpha
                dst[1] = pngData[srcPos + 2]; // Blue
                dst[2] = pngData[srcPos + 1]; // Green
                dst[3] = pngData[srcPos + 0]; // Red
            }
        }


        *subtex = (Tex3DS_SubTexture){
            .width  = 256,
            .height = 256,
            .left   = 0.0f,
            .top    = 1.0f,
            .right  = 256 / (float)256,
            .bottom = 1.0f - (256 / (float)256)
        };

        *image = (C2D_Image){ tex, subtex };
        free(pngData);
        return true;
    }
};

class CTRImage : public CTRGraphic {
public:
    int index = 0;
    C2D_Sprite spr{};
    TrackedSpriteSheet* spriteSheet = nullptr;

    ~CTRImage() {
        if (spriteSheet) {
            spriteSheet->users--;
			TrackedSpriteSheet::Cleanup();
            spriteSheet = nullptr;
        }
    }

    CTRImage(
        const std::string& ap,
        bool display,
        bool los,
        const Vector3& p,
        const Vector3& s,
        const Vector3& r,
        std::string n,
        std::string nspace,
        bool instaLoad,
        Color c,
        int screenIndex = 0,
        int ind = 0
    ) : CTRGraphic(c, display, los, p, s, r, Enums::AssetType::Image, ap, instaLoad, n, nspace, screenIndex),
        index(ind)
    {
        initSprite();
    }

    void ChangeSpriteFromPNG(string content)
    {
        C2D_Image img;
        bool success = C2DImageLoaderC2D::loadPngImage(&img, content);
        if (success)
        {
            if (spriteSheet) {
                spriteSheet->users--;
                TrackedSpriteSheet::Cleanup();
                spriteSheet = nullptr;
            }
            C2D_SpriteFromImage(&spr, img);
        }
    }

    void initSprite() {
        if (assetPath.rfind("romfs:/", 0) != 0) {
            assetPath = "romfs:/gfx/" + assetPath + ".t3x";
        }

        spriteSheet = TrackedSpriteSheet::GetOrCreate(assetPath);

        if (!spriteSheet || spriteSheet->sheet == nullptr) {
            Log::Append("Tried loading sprite: \"" + assetPath + "\", but it wasn't there.");
            return;
        }

        C2D_SpriteFromSheet(&spr, spriteSheet->sheet, index);
        C2D_SpriteSetCenter(&spr, 0.5f, 0.5f);
        C2D_SpriteSetPos(&spr, position.x, position.y);
        C2D_SpriteSetRotationDegrees(&spr, rotation.z);
        C2D_SpriteSetScale(&spr, size.x, size.y);
    }

    void ChangeSprite(const std::string& sname) {
        std::string fullPath = sname;
        if (fullPath.rfind("romfs:/", 0) != 0) {
            fullPath = "romfs:/gfx/" + fullPath + ".t3x";
        }

        if (spriteSheet) {
            spriteSheet->users--;
        }

        assetPath = fullPath;
        initSprite();
    }

    void render() {
        if (visible && spriteSheet && spriteSheet->sheet) {
            C2D_ImageTint tint = GetColor();
            C2D_DrawSpriteTinted(&spr, &tint);
        }
    }

    void renderAt(Vector3 pos2) {
        if (visible && spriteSheet && spriteSheet->sheet) {
            C2D_SpriteSetPos(&spr, pos2.x, pos2.y);
            C2D_ImageTint tint = GetColor();
            C2D_DrawSpriteTinted(&spr, &tint);
            C2D_SpriteSetPos(&spr, position.x, position.y);
        }
    }

    void SetOrigin(Vector2 orig) {
        C2D_SpriteSetCenter(&spr, orig.x, orig.y);
    }

    void UpdateProperties() {
        UpdateLink();
        C2D_SpriteSetPos(&spr, renderPosition.x, renderPosition.y);
        C2D_SpriteSetRotationDegrees(&spr, renderRotation.z);
        C2D_SpriteSetScale(&spr, renderSize.x, renderSize.y);
        GetColor();
    }

    C2D_ImageTint GetColor() {
        C2D_ImageTint tint;
        C2D_Tint colorTint = {C2D_Color32(color.r, color.g, color.b, color.a), color.blend / 255};
        for (int i = 0; i < 4; ++i)
            tint.corners[i] = colorTint;
        return tint;
    }
};

class CTRCamera
{
public:
    bool AutoRender = true;
    bool ZLayering = true;
    Color color;
    CTRScissor Scissoring;
    Vector3 position;
    Vector3 rotation;
    Vector3 size;
    string RenderSpace = "";
	int screenIndex = 0;
	C3D_RenderTarget* top;
	C3D_RenderTarget* bottom;

    CTRCamera(Vector3 p, Vector3 s, Vector3 r, bool AR, Color c, CTRScissor sc, string rs, int sI = 0) :
    AutoRender(AR), color(c), position(p), rotation(r), Scissoring(sc), size(s), RenderSpace(rs), screenIndex(sI) {}

    void PrepareRender()
    {
        switch (screenIndex)
        {
            case 0: // upper screen
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)); // Clear screen
                C2D_SceneBegin(top);
                C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
            break;
            case 1:
                C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)); // Clear screen
                C2D_SceneBegin(bottom);
                C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
            break;
        }
		Scissoring.Set();
    }

    void Render()
    {
        PrepareRender();

        std::vector<GameAsset*> renderables;

        for (GameAsset* asset : GameAssets) {
            if (asset->screenIndex != screenIndex)
                continue;
            if (RenderSpace != "" && asset->Namespace.rfind(RenderSpace, 0) != 0)
                continue;

            if (asset->type == Enums::AssetType::Image || asset->type == Enums::AssetType::ImageFont)
                renderables.push_back(asset);
        }

        if (ZLayering)
        {

            // Step 2: Sort renderables by z-position
            std::sort(renderables.begin(), renderables.end(), [](GameAsset* a, GameAsset* b) {
                float az = 0.0f;
                float bz = 0.0f;

                if (a->type == Enums::AssetType::Image)
                    az = static_cast<CTRImage*>(a)->position.z;
                else if (a->type == Enums::AssetType::ImageFont)
                    az = static_cast<CTRImageFont*>(a)->position.z;

                if (b->type == Enums::AssetType::Image)
                    bz = static_cast<CTRImage*>(b)->position.z;
                else if (b->type == Enums::AssetType::ImageFont)
                    bz = static_cast<CTRImageFont*>(b)->position.z;

                return az < bz;
            });

        }

        for (GameAsset* asset : renderables) {
            RenderObject(asset);
        }
        Scissoring.SetNone();
    }

    void RenderObject(GameAsset* asset)
    {
        if (asset->screenIndex != screenIndex)
                return;
        if (asset->Namespace.rfind(RenderSpace, 0) != 0 && RenderSpace != "")
        {
            return;
        }
        switch (asset->type) {
            case Enums::AssetType::Image: {
                CTRImage* image = static_cast<CTRImage*>(asset);
                if (image) {
                    image->UpdateProperties();
                    image->renderPosition -= position;
                    image->render();
                }
                break;
            }

            case Enums::AssetType::ImageFont: {
                CTRImageFont* font = static_cast<CTRImageFont*>(asset);
                if (font)
                {
                    font->UpdateProperties();
                    font->renderPosition -= position;
                    font->render();
                }
                break;
            }
        }
    }
};

#endif