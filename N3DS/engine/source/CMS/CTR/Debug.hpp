#pragma once

#include <3ds.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include "../../Game.h"
#include <sstream>
#include "FileManager.hpp"

class Log
{
private:
    static std::string DebugLog;

public:
    static inline void Append(const std::string& toAppend)
    {
        DebugLog += toAppend;
    }
    static inline void Subtract(unsigned int count)
    {
        if (count <= DebugLog.length())
            DebugLog.resize(DebugLog.length() - count);
    }
    static inline void Save()
    {
        CTRFiles::MakePath(GetDirectory());
        FILE* file = fopen(GetLocation().c_str(), "w");
        if (file) {
            fputs(Get().c_str(), file);
            fclose(file);
        }
    }
    static inline void Clear()
    {
        DebugLog.clear();
    }
    static inline std::string Get()
    {
        return DebugLog;
    }
    static inline std::string GetDirectory()
    {
        return std::string("sdmc:/")+CTRGame::Author+"/"+CTRGame::Name+"/";
    }
    static inline std::string GetLocation()
    {
        return GetDirectory()+"Log.txt";
    }
};
