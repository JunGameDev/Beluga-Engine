// Copyright (c) 2020 Junyoung Kim

#pragma once
#include <memory>
#include <vector>

#include "Systems/HardwareResource.h"

namespace Bel
{
    class IKeyboard;
    class IMouse;
    class IGameController;

    class IWindow
    {
    public:
        virtual ~IWindow() {}

        virtual bool Initialize(const char* pName, uint32_t width, uint32_t height) = 0;
        virtual bool ProcessEvents() const = 0;
        virtual void* GetNativeWindow() const = 0;

        virtual void NextFrame() = 0;

        virtual bool AttachKeyboard(std::unique_ptr<IKeyboard> pInput) = 0;
        virtual IKeyboard* GetKeyboard() = 0;

        virtual bool AttachMouse(std::unique_ptr<IMouse> pInput) = 0;
        virtual IMouse* GetMouse() = 0;

        virtual bool AttachController(std::unique_ptr<IGameController> pInput) = 0;
        virtual IGameController* GetController() = 0;
    };

    class ISystem
    {
    protected:
        std::unique_ptr<IProcessorInfoReader>   m_pProcessorInfoReader;
        std::unique_ptr<IMemoryInfoReader>      m_pMemoryInfoReader;
        std::unique_ptr<IStorageInfoReader>     m_pStorageInfoReader;
        std::unique_ptr<IGpuInfoReader>         m_pGpuInfoReader;

    public:
        virtual ~ISystem() {}

        static std::unique_ptr<ISystem> Create();
        virtual const char* GetSystemName() const = 0;
        virtual std::unique_ptr<IWindow> CreateSystemWindow(const char* pName, uint32_t width, uint32_t height) = 0;

        virtual bool Initialize() = 0;

        // ===== System Info =====
        virtual bool CheckSystemVersion() = 0;

        // ===== Hardware Info Readers =====
        const IProcessorInfoReader& GetProcessorInfoReader()    { return *(m_pProcessorInfoReader.get());  }
        const IMemoryInfoReader&    GetMemoryInfoReader()       { return *(m_pMemoryInfoReader.get());     }
        const IStorageInfoReader&   GetStorageInfoReader()      { return *(m_pStorageInfoReader.get());    }
        const IGpuInfoReader&       GetGpuInfoReader()          { return *(m_pGpuInfoReader.get());        }

        // ===== File & Directory =====
        virtual bool CheckFileModified(const char* pPath) = 0;
        virtual void SetCurrentWorkingDirectory(const std::string& path) = 0;
        virtual std::vector<std::string> GetAllFiles(const std::string& path) = 0;
        virtual void GetFilesRecursive(const std::string& path, const std::string& directory, std::vector<std::string>& files) = 0;

        // ===== Pop Up message =====
        virtual bool ShowPopUpMessage(const wchar_t* pTitle, const wchar_t* pContext) = 0;

        // ===== Clock =====
        const std::string GetCurrentTimeStr() const;
        
        // ===== Loadin Files =====
        virtual void ReadTextFile(const std::string& path) {};
    };
}

