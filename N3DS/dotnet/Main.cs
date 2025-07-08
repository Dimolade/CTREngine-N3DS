using System.IO;
using Newtonsoft.Json;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.ComponentModel;
using Newtonsoft.Json.Linq;
using CTR;

namespace CTR.Projects
{
    public class Project
    {
        public string name;
        public string path;
        public string ctrVersion;
        public string platformID;
        public string csmVersion;
        public string ctrProjPath;

        public Project(string name, string path, string ctrVersion, string platformID, string csmVersion, string ctrProjPath)
        {
            this.name = name;
            this.path = path;
            this.ctrVersion = ctrVersion;
            this.platformID = platformID;
            this.csmVersion = csmVersion;
            this.ctrProjPath = ctrProjPath;
        }
    }
}

namespace CTR
{
    public class CTRIFChar
    {
        public char c;
        public string filePath;
        public int Width;
        public int Height;
        
        public CTRIFChar(char c, string filePath)
        {
            this.c = c;
            this.filePath = filePath;
        }
    }
    public class CTRIF
    {
        public List<CTRIFChar> Chars;
        public CTRIFChar defaultChar;

        public CTRIF()
        {
            Chars = new List<CTRIFChar>();
        }
    }

    public class CTRGD
    {
        public string Name;
        public string Author;
        public string Description;
        public string Version;
        public string IconPath;
    }
}

public static class PlatformFunctions
{

    // Basic Info
    public static string id = "Dimolade.N3DS"; // Unique identifier for the platform
    public static string name = "N3DS"; // Display name of the platform (e.g., "Nintendo DS", "Android")
    public static string info = "Early Development N3DS for CTR Engine";
    public static string sourceDirectory = "N3DS/engine/source/";

    // Compatibility & Features
    public static bool buildSupported = true; // If the platform supports building games
    public static bool buildRomSupported = false; // If the platform supports ROM building
    public static bool hasTouchscreen = true; // Whether the platform has a touchscreen
    public static bool supportsAudio = true; // Whether the platform supports audio playback
    public static bool supportsNetworking = true; // Whether the platform supports online/networking features

    // Display Info
    public static int numOfScreens = 2; // Number of screens the platform has
    public static int primaryScreenIndex = 0; // Index of the primary screen
    public static int screenWithTouchscreen = 1; // Index of the screen with touchscreen if applicable
    public static int[][] ScreenSizes = new int[][]
    {
        new int[] {400, 240},
        new int[] {320, 240}
    };

    // Input Info
    public static bool supportsGamepad = true; // Whether the platform supports gamepad input
    public static bool supportsKeyboard = false; // Whether the platform supports keyboard input
    public static bool supportsMouse = false;

    // Storage Info
    public static bool supportsSaveData = true; // Whether the platform supports saving/loading data
    public static long maxSaveSize = 1000000; // Maximum size of save data in bytes
    public static string assetStoragePath = "romfs:/";
    public static string imagePrefix = "gfx/";
    public static string soundPrefix = "snd/";
    public static string otherPrefix = "otr/";
    public static string defaultImageType = "t3x";
    public static string defaultSoundType = "ogg";

    // Platform Limitations
    public static int maxTextureSize = 1024; // Maximum texture resolution allowed
    public static int maxAudioChannels = 2; // Maximum audio channels supported

    public static void DeleteAllInDir(string dir, string basePath)
    {
        foreach (string file in Directory.GetFiles(basePath+dir))
        {
            File.Delete(file);
        }

        // Delete all subdirectories
        foreach (string subDirectory in Directory.GetDirectories(basePath+dir))
        {
            if (Path.GetFileName(subDirectory) == "CTR")
            {
                continue;
            }
            Directory.Delete(subDirectory, true); // true: delete subdirectories recursively
        }
    }

    static string EscapeForCpp(string s)
    {
        return s
            .Replace("\\", "\\\\")
            .Replace("\"", "\\\"")
            .Replace("\r", "")             // remove carriage returns (optional)
            .Replace("\n", "\\n");         // escape newlines
    }

    // Functions
    public static void BuildPlatform(string pathToCTRProj)
    {
        Console.WriteLine("BUILDING FROM N3DS Platform");
        Console.WriteLine("Path to CTR Project for N3DS Platform: "+pathToCTRProj);
        string basePath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), "CartridgeTiltRetro/N3DS/");
        string dataPath = Path.Combine(basePath,"engine/data/");
        CTR.Projects.Project p;
        try
        {
            string jsonContent = File.ReadAllText(pathToCTRProj);
            p = JsonConvert.DeserializeObject<CTR.Projects.Project>(jsonContent);
            if (p == null)
            {
                Console.WriteLine("Error: Deserialization returned null. Check the JSON format.");
                return;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error deserializing JSON: {ex.Message}");
            return;
        }
        string buildFolder = Path.Combine(p.path,"Build/");
        string assetFilePath = Path.Combine(p.path,"Assets/");
        string[] Files = GetAllFiles(Path.Combine(p.path,"Assets/"));
        string[] CppFiles = GetAllFiles(buildFolder);
        DeleteAllInDir("engine/source/CMS", basePath);
        DeleteAllInDir("engine/romfs/snd", basePath);
        DeleteAllInDir("engine/romfs/otr", basePath);
        DeleteAllInDir("engine/gfx", basePath);
        List<(CTRIF, string)> imageFonts = new List<(CTRIF, string)>();
        string ctrgdP = Path.Combine(assetFilePath, "GameData.ctrgd");
        Console.WriteLine("Checking Existence of "+ctrgdP);
        string gameCpp = @"
#include ""Game.h""
using namespace std;

string CTRGame::Name = ""Game"";
string CTRGame::Author = ""Person"";
string CTRGame::Description = ""Description"";
string CTRGame::Version = ""0.0.1"";
";
        CTRGD gd = (File.Exists(ctrgdP) ? JsonConvert.DeserializeObject<CTRGD>(File.ReadAllText(ctrgdP)) : null);
        if (gd != null)
        {

            gameCpp = $@"
#include ""Game.h""
using namespace std;

string CTRGame::Name = ""{gd.Name}"";
string CTRGame::Author = ""{gd.Author}"";
string CTRGame::Description = ""{EscapeForCpp(gd.Description)}"";
string CTRGame::Version = ""{gd.Version}"";
";
            File.WriteAllText(Path.Combine(dataPath, "name.txt"), gd.Name);
            File.WriteAllText(Path.Combine(dataPath, "description.txt"), gd.Description);
            File.WriteAllText(Path.Combine(dataPath, "author.txt"), gd.Author);
            File.Copy(Path.Combine(p.path,gd.IconPath), Path.Combine(dataPath,"Icon.png"), true);
        }
        Console.WriteLine("Writing to: "+Path.Combine(basePath, "engine/source/")+"Game.cpp");
        File.WriteAllText(Path.Combine(basePath, "engine/source/")+"Game.cpp", gameCpp);
        try
        {
            int i1 = 0;
            foreach (string f in Files)
            {
                if (f.EndsWith(".png") || f.EndsWith(".jpg") || f.EndsWith(".bmp"))
                {
                    File.Copy(f, basePath+"engine/gfx/"+Path.GetFileName(f), true);
                    File.WriteAllText(basePath+"engine/gfx/"+Path.GetFileNameWithoutExtension(f)+".t3s","--atlas -f rgba8888 -z auto\n"+Path.GetFileName(f));
                }
                else if (f.EndsWith(".ctrif"))
                {
                    // CTR Image Font
                    CTRIF font = JsonConvert.DeserializeObject<CTRIF>(File.ReadAllText(f));
                    string currentt3s = "--atlas -f rgba8888 -z auto\n";
                    string defaultfCHName = $"ctriffont{i1.ToString()}_"+Path.GetFileName(font.defaultChar.filePath);
                    File.Copy(font.defaultChar.filePath, basePath+"engine/gfx/"+defaultfCHName, true);
                    currentt3s += defaultfCHName+"\n";
                    foreach (CTRIFChar ch in font.Chars)
                    {
                        string fp = ch.filePath;
                        string name = $"ctriffont{i1.ToString()}_"+Path.GetFileName(fp);
                        currentt3s += name+"\n";
                        File.Copy(fp, basePath+"engine/gfx/"+name, true);
                    }
                    File.WriteAllText(basePath+"engine/gfx/"+Path.GetFileNameWithoutExtension(f)+".t3s", currentt3s);
                    imageFonts.Add((font, basePath+"engine/gfx/"+Path.GetFileNameWithoutExtension(f)+".t3s"));
                }
                else if (f.EndsWith(".ogg"))
                {
                    File.Copy(f, basePath+"engine/romfs/snd/"+Path.GetFileName(f), true);
                }
                i1++;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"An error occurred: {ex.Message}\n{ex.Source}\n{ex.StackTrace}");
        }

        string Constructor = "";
        string includes = "";
        string eps = "";

        foreach (string f in CppFiles)
        {
            string fn = Path.GetFileName(f);
            if (fn == "GeneratedSceneObjects.cpp")
            {
                Constructor = File.ReadAllText(f);
            }
            else if (fn == "GeneratedIncludes.cpp")
            {
                includes = File.ReadAllText(f);
            }
            else if (fn == "EntryPoints")
            {
                eps = File.ReadAllText(f);
            }
            else
            {
                File.Copy(f,basePath+"engine/source/CMS/"+Path.GetFileName(f), true);
            }
        }

        List<string> constructorsblbyfonts = new List<string>();

        foreach (var (font, path) in imageFonts)
        {
            string bfcVector = "{";
            string fname = "romfs:/gfx/"+Path.GetFileNameWithoutExtension(path)+".t3x";
            int i0 = 1;
            foreach (CTRIFChar ch in font.Chars)
            {
                string cchar = ch.c.ToString();
                if (ch.c == '\'')
                {
                    cchar = "\\'";
                    Console.WriteLine("BLOBBY FONT DEBUG: Matched single quote in Font");
                }
                bfcVector += "BlobbyFontChar('"+cchar+"',\""+fname+"\","+i0.ToString()+
                ","+ch.Width+","+ch.Height+")";

                if (i0 != font.Chars.Count)
                {
                    bfcVector += ",";
                }

                i0++;
            }
            bfcVector += "}";
            CTRIFChar ch1 = font.defaultChar;
            string cchar1 = ch1.c.ToString();
            if (ch1.c == '\'')
            {
                cchar1 = "\\'";
                Console.WriteLine("BLOBBY FONT DEBUG: Matched single quote in Font");
            }
            string defaultChar = "BlobbyFontChar('"+cchar1+"',\""+fname+"\",0"+
                ","+ch1.Width+","+ch1.Height+")";
            constructorsblbyfonts.Add(
                "new BlobbyFont(\""+Path.GetFileNameWithoutExtension(path)+"\","+
                bfcVector+","+defaultChar+",\""+fname+"\")"
            );
        }

        string addString0 = "";

        int i = 0;
        foreach (string s in constructorsblbyfonts)
        {
            addString0 += s;
            if (i+1 != constructorsblbyfonts.Count)
            {
                addString0 += ",";
            }
            i++;
        }

        string ctrblobbyfontsconst =
        "vector<BlobbyFont*> CTRBlobbyFonts::fonts;\nvoid CTRBlobbyFonts::InitFonts() {\n"+
        $"CTRBlobbyFonts::fonts.reserve({imageFonts.Count.ToString()});\n"+
        "CTRBlobbyFonts::fonts = {\n"+
        addString0+"};\n}";

        string command = "ls"; // Use "dir" for Windows
        string directory = "/home/user"; // Change this to the directory you want

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            command = "dir";

        string currentMainCPP = "";
        currentMainCPP += @"
#include <citro2d.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include ""CTR/Enums.h""
#include ""CTR/Classes.h""
#include ""CTR/CTRScene.h""
#include ""BlobbyFont.h""
#include ""BlobbyAudio.h""
#include ""CMS/CTR/Time.h""
#include ""CMS/CTR/Debug.hpp""
#include <vector>
#include <3ds.h>
"+"\n"+includes+@"

// Dimolade File version 5

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240
using namespace std;
using namespace Vectors;
        ";

        currentMainCPP += "\n\n"+Constructor+"\n\n";
        currentMainCPP += ctrblobbyfontsconst+"\n\n";
        currentMainCPP += @"
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

    "+eps+@"

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
                    //std::cout << ""Unknown asset type."" << std::endl;
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
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)); // Clear screen
        C2D_SceneBegin(top);
        C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

        for (GameAsset* asset : GameAssets) {
            if (asset->screenIndex == 1)
                continue;
            switch (asset->type) {
                case Enums::AssetType::Image: {
                    CTRImage* image = static_cast<CTRImage*>(asset);
                    if (image) {
                        image->render();
                    }
                    break;
                }

                case Enums::AssetType::Font:
                    // Add font rendering logic here
                    break;

                case Enums::AssetType::ImageFont: {
                    CTRImageFont* font = static_cast<CTRImageFont*>(asset);
                    if (font)
                    {
                        font->render();
                    }
                    break;
                }

                default:
                    //std::cout << ""Unknown asset type."" << std::endl;
                    break;
            }
        }

        C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)); // Clear screen
        C2D_SceneBegin(bottom);
        C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

        for (GameAsset* asset : GameAssets) {
            if (asset->screenIndex == 0)
                continue;
            switch (asset->type) {
                case Enums::AssetType::Image: {
                    CTRImage* image = static_cast<CTRImage*>(asset);
                    if (image->screenIndex == 2)
                    {
                        image->renderAt({image->position.x-40, image->position.y-240, image->position.z});
                    }
                    else
                    {
                        image->render();
                    }
                    break;
                }
                case Enums::AssetType::Sound:
                    // Add sound playback logic here
                    break;

                case Enums::AssetType::Font:
                    // Add font rendering logic here
                    break;

                case Enums::AssetType::ImageFont: {
                    CTRImageFont* font = static_cast<CTRImageFont*>(asset);
                    if (font)
                    {
                        font->render();
                    }
                    break;
                }

                default:
                    //std::cout << ""Unknown asset type."" << std::endl;
                    break;
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
        ";

        File.WriteAllText(basePath+"engine/source/main.cpp", currentMainCPP);

        string output = RunCommandInDirectory("make clean && make", basePath+"engine/");
        Console.WriteLine("Make output: "+output);
    }

    static string RunCommandInDirectory(string command, string workingDirectory)
    {
        ProcessStartInfo psi = new ProcessStartInfo
        {
            FileName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? "cmd.exe" : "/bin/bash",
            Arguments = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? $"/c {command}" : $"-c \"{command}\"",
            WorkingDirectory = workingDirectory,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };

        using (Process process = Process.Start(psi))
        {
            string output = process.StandardOutput.ReadToEnd();
            string error = process.StandardError.ReadToEnd();
            process.WaitForExit();

            return string.IsNullOrEmpty(error) ? output : $"Error: {error}";
        }
    }

    public static string[] GetAllFiles(string directoryPath)
    {
        return Directory.GetFiles(directoryPath, "*.*", SearchOption.AllDirectories);
    }

    public static void OnBuiltROM(string pathToBuiltROM)
    {
        // Do something when the rom finishes building.
    }
}
