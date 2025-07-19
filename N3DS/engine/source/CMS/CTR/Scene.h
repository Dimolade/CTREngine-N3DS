#ifndef SCENE_H
#define SCENE_H

/*#include "../../CTR/Classes.h"*/
/*#include "../../CTR/Enums.h"
#include "../../CTR/CTRScene.h"*/
#include <string>

class GameAsset;
class CTRImage;
class CTRImageFont;
class CTRSound;
class CTRCamera;

class Scene {  // Added Scene class
public:
    static GameAsset* GetSceneObject(const std::string& name, const std::string& Namespace);
    static CTRImage* GetCTRImage(const std::string& name, const std::string& Namespace);
    static CTRImageFont* GetCTRImageFont(const std::string& name, const std::string& Namespace);
    static CTRSound* GetCTRSound(const std::string& name, const std::string& Namespace);
    static void AddSceneObject(GameAsset* asset);

    static void AddCamera(CTRCamera* cam);

    //const
    static CTRImage* ConstCTRImage(const std::string& name, const std::string& Namespace, const std::string& spriteName);
    static CTRSound* ConstCTRSound(const std::string& name, const std::string& Namespace, const std::string& soundName);
    static CTRImageFont* ConstCTRImageFont(const std::string& name, const std::string& Namespace, const std::string& spriteName, const std::string& text = "");
    static CTRCamera* ConstCTRCamera(const int& screenIndex);
};

#endif // SCENE_H
