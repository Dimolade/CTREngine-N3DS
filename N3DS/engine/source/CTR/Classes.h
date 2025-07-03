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

    //CTRAxyz(Vector3 p, Vector3 s, Vector3 r) : position(p), size(s), rotation(r) {}
    CTRAxyz(Vector3 p, Vector3 s, Vector3 r, Enums::AssetType t, string ap, bool il = false, string n = "default", string nspace = "default", int sI = 0)
    : position(p), size(s), rotation(r), GameAsset(t,ap,il,n,nspace,sI) {}
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
            assetPath = ("romfs:/snd/"+assetPath+".ogg");
        }
        blobbyAudio.LoadClip(ap);
    }

    void SetClipPCM(const int16_t* pcmData, size_t sampleCount, int sampleRate = 22050, bool stereo = false)
    {
        blobbyAudio.LoadPCM(pcmData, sampleCount, sampleRate, stereo);
    }

    void SetClipPCM(const std::vector<int16_t>& pcmData, int sampleRate = 22050, bool stereo = false)
    {
        blobbyAudio.LoadPCM(pcmData, sampleRate, stereo);
    }

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

class CTRImage : public CTRGraphic {
public:
    int index = 0;
    C2D_Sprite spr{};
    TrackedSpriteSheet* spriteSheet = nullptr;

    ~CTRImage() {
        if (spriteSheet) {
            spriteSheet->users--;
			TrackedSpriteSheet::Cleanup();
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
        spriteSheet = TrackedSpriteSheet::GetOrCreate(assetPath);

        if (!spriteSheet || spriteSheet->sheet == nullptr) {
            Log::Append("Tried loading sprite: \"" + assetPath + "\", but it wasn't there.");
            return;
        }

        C2D_SpriteFromSheet(&spr, spriteSheet->sheet, 0);
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
        C2D_SpriteSetPos(&spr, position.x, position.y);
        C2D_SpriteSetRotationDegrees(&spr, rotation.z);
        C2D_SpriteSetScale(&spr, size.x, size.y);
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

#endif