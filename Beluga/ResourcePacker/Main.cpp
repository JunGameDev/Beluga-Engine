#include <Resources/Resource.h>
#include <Systems/System.h>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <chrono>

#if defined(_WIN32)
#include <Windows.h>
#endif 

using namespace Bel;

int main(int argc, char* args[])
{
    if (argc < 2)
    {
        return 1;
    }
    std::string path = args[1];

    auto pSystem = ISystem::Create();
    auto files = pSystem->GetAllFiles(path);

    ZlibFile resources;
    bool foundFileModified = false;

    for (auto& file : files)
    {
        std::string resourcePath = path + "/" + file;
        std::fstream resourceFile(resourcePath, std::ios_base::in | std::ios_base::binary);
        if (resourceFile.is_open())
        {
            resourceFile.seekg(0, resourceFile.end);
            std::size_t fileSize = static_cast<size_t>(resourceFile.tellg());
            resourceFile.seekg(0, resourceFile.beg);

            std::vector<char> data(fileSize);
            resourceFile.read(data.data(), fileSize);
            resources.AddResource(file, std::move(data));
        }
    }

    //if(foundFileModified)
        resources.Save(args[2]);

    return 0;
}
