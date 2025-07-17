
#include <citro2d.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include "CTR/Enums.h"
#include "CTR/Classes.h"
#include "CTR/CTRScene.h"
#include "BlobbyFont.h"
#include "BlobbyAudio.h"
#include "CMS/CTR/Time.h"
#include "CMS/CTR/Debug.hpp"
#include <vector>
#include <3ds.h>

#include "CMS/RenderTest.hpp"


// Dimolade File version 5

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240
using namespace std;
using namespace Vectors;
        

std::vector<GameAsset*> GameAssets;
void CTRScene::InitScene() {
GameAssets.reserve(1);
GameAssets = {
new GameAsset(Enums::AssetType::Script, "RenderTest",true,"Script","Scripts",0)
};
};

vector<BlobbyFont*> CTRBlobbyFonts::fonts;
void CTRBlobbyFonts::InitFonts() {
CTRBlobbyFonts::fonts.reserve(0);
CTRBlobbyFonts::fonts = {
};
}


// Initialize libraries and setup
void initLibraries() {
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    fsInit();

    ndspInit();

    CTRScene::InitScene();
    CTRBlobbyFonts::InitFonts();

    RenderTest* Script = new RenderTest();


    for (GameAsset* asset : GameAssets) {
            switch (asset->type) {
                case Enums::AssetType::Image: {
                    CTRImage* image = static_cast<CTRImage*>(asset);
                    if (image) {
                        //image->initSprite(); // does it auto now.
                    }
                    break;
                }
                case Enums::AssetType::Sound: {
                    CTRSound* snd = static_cast<CTRSound*>(asset);
                    if (snd) {
                        snd->blobbyAudio.LoadClip(snd->assetPath);
                        if (snd->playOnLoad)
                        {
                            snd->blobbyAudio.Play();
                        }
                    }
                    break;
                }

                case Enums::AssetType::Font:
                    // Add font rendering logic here
                    break;

                case Enums::AssetType::ImageFont: {
                    CTRImageFont* font = static_cast<CTRImageFont*>(asset);
                    if (font) {
                        BlobbyFont* bf = nullptr;
                        for (BlobbyFont* bfont : CTRBlobbyFonts::fonts)
                        {
                            if (bfont->id == asset->assetPath)
                            {
                                bf = bfont;
                                font->textH = new BlobbyText(font->Text, bf, font->position.x, font->position.y, font->size.x, font->size.y, font->rotation.z, font->color);
                            }
                        }
                    }
                    break;
                }

                default:
                    //std::cout << "Unknown asset type." << std::endl;
                    break;
            }
        }
}

// Cleanup libraries
void cleanupLibraries() {
    Log::Save();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    fsExit();

    ndspExit();

    for (GameAsset* asset : GameAssets) {
		switch (asset->type) {
			case Enums::AssetType::Sound: {
				CTRSound* sound = static_cast<CTRSound*>(asset);
				if (sound) {
					sound->Stop();
				}
				break;
			}
		}
	}
	
	for (BlobbyFont* bfont : CTRBlobbyFonts::fonts)
	{
		delete bfont;
	}

    for (GameAsset* asset : GameAssets) {
        delete asset;
    }

    vector<GameAsset*>().swap(GameAssets);
	vector<BlobbyFont*>().swap(CTRBlobbyFonts::fonts);
}

// Main function
int main(int argc, char* argv[]) {
    // Initialize libraries
    initLibraries();

    // Create screen target
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    for (CTREntry* entry : entryPoints) {
        entry->EntryPoint();
    }
    // Main loop
    float lastTime = osGetTime();
    while (aptMainLoop()) {
        hidScanInput();

        u64 currentTime = osGetTime();
        Time::deltaTime = (currentTime - lastTime) / 1000.0f; // convert ms to seconds
        lastTime = currentTime;
        //u32 kDown = hidKeysDown();
        //if (kDown & KEY_START) break;  // Exit on Start button

        // Render frame
        for (CTREntry* entry : entryPoints) {
            entry->OnFrame();
        }
        Coroutiner::Update();

        for (CTRCamera* camera : CAMERAS)
        {
            if (camera->AutoRender)
            {
                camera->Render();
            }
        }

        for (CTREntry* entry : entryPoints) {
            entry->AfterFrame();
        }

        C3D_FrameEnd(0);
    }

    // Cleanup
    cleanupLibraries();

    return 0;
}
        