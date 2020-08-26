// Copyright (c) 2020 Junyoung Kim

//---------------------------------------------------------
// Standard library
//---------------------------------------------------------
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <chrono>
#include <iostream>
#include <filesystem>

//---------------------------------------------------------
// Windows Specific library
//---------------------------------------------------------
#if defined(_WIN32)
#include <Windows.h>
#endif 

//---------------------------------------------------------
// Beluga Engine
//---------------------------------------------------------
#include <Resources/Resource.h>
#include <Systems/System.h>
#include <Scripting/Scripting.h>

using namespace Bel;

int main(int argc, char* args[])
{
    if (argc < 2)
    {
        std::cout << "Didn't get any input" << std::endl;
        return 1;
    }
    
    std::string path = args[1];

    ScriptingManager scriptingManager;
    scriptingManager.Initialize();

    scriptingManager.CreateTable();
    scriptingManager.SetGlobal("g_logic");    

    // Get every files' path
    auto pSystem = ISystem::Create();
    
    // Create resource file
    std::string  scriptName = std::string(args[0]) + "/../GluaGen.Lua";
    //std::string  scriptName = "GluaGen.Lua";
    std::fstream resourceFile(scriptName, std::ios_base::in | std::ios_base::binary);

    std::shared_ptr<ResourceHandle> pScript;
    if (resourceFile.is_open())
    {
        resourceFile.seekg(0, resourceFile.end);
        std::size_t fileSize = static_cast<size_t>(resourceFile.tellg());
        resourceFile.seekg(0, resourceFile.beg);

        std::vector<char> data(fileSize);
        resourceFile.read(data.data(), fileSize);
        pScript = std::make_shared<ResourceHandle>(Resource(scriptName), std::move(data));
    }

    resourceFile.close();

    // TODO
    //  [] Make it works only when the file changes detected.

    // Run GluaGen.lua
    scriptingManager.RunScript(pScript);
    
    auto files = pSystem->GetAllFiles(path);
    pSystem->SetCurrentWorkingDirectory(path);

    std::vector<std::string> filesToBind;

    for (auto& file : files)
    {
        // If it isn't a header file, skip it.
        if (file.substr(file.size() - 2, file.size()) != ".h")
            continue;

        std::string genFile = file.substr(0, file.size() - 2) +".gen";

        // Check is there a gen file in directory.
        auto iter = std::find(files.begin(), files.end(), genFile);
        if (iter != files.end())
        {

            struct stat headerFileInfo;
            struct stat genFileInfo;


            if (stat(file.data(), &headerFileInfo) != 0 || stat(iter->data(), &genFileInfo))
            {
                continue;
            }
            auto lastHeaderModified = std::chrono::system_clock::from_time_t(headerFileInfo.st_mtime);
            auto lastGenModified = std::chrono::system_clock::from_time_t(genFileInfo.st_mtime);

            char buf[256];

            std::time_t header = std::chrono::system_clock::to_time_t(lastHeaderModified);
            std::time_t gen = std::chrono::system_clock::to_time_t(lastGenModified);
            std::cout << "\t+-------------------------------------------------------+" << std::endl;
            std::cout << "\t\t\t\t\t\t" << file << std::endl;
            std::cout << "\t+-------------------------------------------------------+" << std::endl;

            ctime_s(buf, sizeof(buf), &gen);
            std::string str = buf;
            std::cout << "\tGen File Modified: \t\t\t\t" << str;

            ctime_s(buf, sizeof(buf), &header);
            str = buf;
            std::cout << "\tHeader File Status Modified: \t" << str;

            auto gap = std::chrono::duration_cast<std::chrono::minutes>(lastHeaderModified - lastGenModified);
            std::cout << "\tGap: \t\t\t\t\t\t\t" << std::abs(gap.count()) << " minutes." << std::endl;

            if (gap.count() < 0 || gap.count() > 1)
            {
                std::cout << "\t\t\t\t\t\t\t\t\t[ Pass ]\n\n";
                continue;
            }
            std::cout << "\t\t\t\t\t\t\t\t\t[ Glue ]\n\n";
            std::string fullPath = path + "\\" + file;
            filesToBind.emplace_back(fullPath);
        }       
    }

    for (auto& file : filesToBind)
    {
        std::cout << file.c_str() << std::endl;
        scriptingManager.StartFunction("Glue");
        scriptingManager.PushString(file.c_str());
        scriptingManager.CallFunction(0);
    }
    
    system("pause");
    return 0;
}