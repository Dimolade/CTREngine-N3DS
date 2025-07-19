#include "Scene.h"
#include "../../CTR/Classes.h"
#include "../../CTR/Enums.h"
#include "../../CTR/CTRScene.h"

GameAsset* Scene::GetSceneObject(const std::string& name, const std::string& Namespace)
{
    for (auto* asset : GameAssets) {
        if (asset->name == name && asset->Namespace == Namespace) {
            return asset;
        }
    }
    return nullptr;
}

CTRImage* Scene::GetCTRImage(const std::string& name, const std::string& Namespace)
{
    for (auto* asset : GameAssets) {
        if (asset->name == name && asset->Namespace == Namespace) {
            return static_cast<CTRImage*>(asset);
        }
    }
    return nullptr;
}

CTRImageFont* Scene::GetCTRImageFont(const std::string& name, const std::string& Namespace)
{
    for (auto* asset : GameAssets) {
        if (asset->name == name && asset->Namespace == Namespace) {
            return static_cast<CTRImageFont*>(asset);
        }
    }
    return nullptr;
}

CTRSound* Scene::GetCTRSound(const std::string& name, const std::string& Namespace)
{
    for (auto* asset : GameAssets) {
        if (asset->name == name && asset->Namespace == Namespace) {
            return static_cast<CTRSound*>(asset);
        }
    }
    return nullptr;
}

void Scene::AddSceneObject(GameAsset* asset)
{
    GameAssets.push_back(asset);
}

void Scene::AddCamera(CTRCamera* cam)
{
    CAMERAS.push_back(cam);
}

CTRImage* Scene::ConstCTRImage(const std::string& name, const std::string& Namespace, const std::string& spriteName)
{
    CTRImage* img = new CTRImage(spriteName, true, false, Vector3(0,0,0), Vector3(1,1,1), Vector3(0,0,0), name, Namespace, true, {255,255,255,255,0.0f} );
    img->UpdateProperties();
    return img;
}

CTRSound* Scene::ConstCTRSound(const std::string& name, const std::string& Namespace, const std::string& soundName)
{
    CTRSound* snd = new CTRSound(name,Namespace,soundName,0,true,false);
    return snd;
}

CTRImageFont* Scene::ConstCTRImageFont(const std::string& name, const std::string& Namespace, const std::string& spriteName, const std::string& text)
{
    CTRImageFont* font = new CTRImageFont(spriteName, true, false, Vector3(0,0,0), Vector3(1,1,1), Vector3(0,0,0), name, Namespace, true, {255,255,255,255,0.0f}, 0, text);
    if (font->assetPath.rfind("romfs:/", 0) != 0)
    {
        font->assetPath = ("romfs:/gfx/"+font->assetPath+".t3x");
    }
    BlobbyFont* bf = nullptr;
    for (BlobbyFont* bfont : CTRBlobbyFonts::fonts)
    {
        if (bfont->id == font->assetPath)
        {
            bf = bfont;
            font->textH = new BlobbyText(font->Text, bf, font->position.x, font->position.y, font->size.x, font->size.y, font->rotation.z, font->color);
        }
    }
    return font;
}

CTRCamera* Scene::ConstCTRCamera(const int& screenIndex)
{
    CTRCamera* ctrcam = new CTRCamera(Vector3(0,0,0), Vector3(0,0,0), Vector3(0,0,0), true, {0,0,0,255,0.0f}, CTRScissor(Enums::CTRScissorMode::None, Vector2(0,0), Vector2(0,0)), "");
    return ctrcam;
}
