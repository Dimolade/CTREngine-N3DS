#pragma once

#include <iostream> // For std::cout, std::cerr
#include <fstream>  // For std::ifstream

using namespace std;

enum class CTRPathType
{
    Full,
    Root,
    Game
};

class CTRFiles
{
private:

    static string GetPath(CTRPathType typ, string path)
    {
        switch (typ)
        {
            case CTRPathType::Full:
                return string;
            break;
            case CTRPathType::Root:
                return string("sdmc:/")+string;
            break;
            case CTRPathType::Game:
                MakePath(Log::GetDirectory());
                return Log::GetDirectory() + path ;
            break;
        }
    }

public:
    static void MakePath(const std::string& fullPath)
    {
        std::string path;
        std::stringstream ss(fullPath);
        std::string segment;

        while (std::getline(ss, segment, '/')) {
            if (segment.empty() || segment.find('.') != std::string::npos)
                continue;

            path += segment + "/";

            mkdir(path.c_str(), 0777);
        }
    }

    static string ReadText(string path, CTRPathType typ)
    {
        std::ifstream inputFile(GetPath(typ, path), std::ios::in); // Reading from the file we just wrote to

        if (inputFile.is_open()) {
            std::string line;
            string full;

            while (std::getline(inputFile, line)) {
                full += line + "\n";
            }

            inputFile.close();
            return full;
        } else {
            return null;
        }
        return null;
    }

    static string ReadTextBytes(string path, CTRPathType typ)
    {
        std::ifstream inputFile(GetPath(typ, path), std::ios::in); // Reading from the file we just wrote to

        if (inputFile.is_open()) {
            std::string pngFileString((std::istreambuf_iterator<char>(inputFile)),
                               std::istreambuf_iterator<char>());

            inputFile.close();
            return pngFileString;
        } else {
            return null;
        }
        return null;
    }
};
