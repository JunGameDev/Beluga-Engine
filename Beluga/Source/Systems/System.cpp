#include <Core/Layers/ApplicationLayer.h>

#if defined(_WIN32)
#include <Windows.h>
#include <VersionHelpers.h>
#endif

#include <tchar.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <iomanip>
#include <iostream>
#include <SDL.h>

#include "Parshing/tinyxml2.h"
#include "Input/Input.h"

using namespace Bel;
using namespace tinyxml2;
using SeverityLevel = Logging::SeverityLevel;

extern std::string WStrToStr(const std::wstring& kWstr);
extern std::wstring StrToWStr(const std::string& kStr);

class SDLWindow : public IWindow
{
private:
    bool m_initialized;
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_pSDLWindow;
    std::unique_ptr<IKeyboard> m_pKeyboard;
    std::unique_ptr<IMouse> m_pMouse;
    std::unique_ptr<IGameController> m_pController;
    std::unordered_map<int, int> m_keyMap;
public:
    SDLWindow()
        : m_initialized(false)
        , m_pSDLWindow(nullptr, nullptr)
    {
    }

    virtual void* GetNativeWindow() const override { return m_pSDLWindow.get(); }
    virtual ~SDLWindow() override
    {
        m_pSDLWindow = nullptr;
        if (m_initialized)
        {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }

        SDL_Quit();
    }

    virtual bool Initialize(const char* pName, uint32_t width, uint32_t height) override
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            LOG_ERROR("Unable to intialize SDL Graphics");
            LOG_ERROR(SDL_GetError());

            return false;
        }

        m_initialized = true;

        // ==== Keyboard Mapping ====
        m_keyMap =
        {
            {SDL_SCANCODE_LCTRL,        IKeyboard::kCTRL_L},
            {SDL_SCANCODE_RCTRL,        IKeyboard::kCTRL_R},
            {SDL_SCANCODE_SPACE,        IKeyboard::kSpace},
            {SDL_SCANCODE_RIGHT,        IKeyboard::kArrowRight},
            {SDL_SCANCODE_LEFT,         IKeyboard::kArrowLeft},
            {SDL_SCANCODE_DOWN,         IKeyboard::kArrowDown},
            {SDL_SCANCODE_UP,           IKeyboard::kArrowUp},
            {SDL_SCANCODE_LEFTBRACKET,  IKeyboard::kLeftBracket},
            {SDL_SCANCODE_RIGHTBRACKET, IKeyboard::kRightBracket},
            {SDL_SCANCODE_ESCAPE,       IKeyboard::kEsc},
            {SDL_SCANCODE_SPACE,        IKeyboard::kSpace}
        };

        for (unsigned int i = SDL_SCANCODE_A; i <= SDL_SCANCODE_Z; ++i)
        {
            m_keyMap.insert(std::pair<int, int>(i, (i - SDL_SCANCODE_A + IKeyboard::kA)));
        }

        for (unsigned int i = SDL_SCANCODE_1; i <= SDL_SCANCODE_0; ++i)
        {
            m_keyMap.insert(std::pair<int, int>(i, (i - SDL_SCANCODE_A + IKeyboard::kA)));
        }
        // ==== Keyboard Mapping end ====

        m_pSDLWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>
            (
                SDL_CreateWindow(pName,
                    SDL_WINDOWPOS_CENTERED,
                    SDL_WINDOWPOS_CENTERED,
                    width,
                    height,
                    SDL_WINDOW_OPENGL),
                &SDL_DestroyWindow
                );

        if (m_pSDLWindow == nullptr)
        {
            LOG_ERROR("Unable to intialize SDL Window");
            LOG_ERROR(SDL_GetError());

            return false;
        }

        return true;
    }

    virtual bool ProcessEvents() const override
    {
        //TODO:
        //  [] Add functionality to detect new input device connection.

        SDL_Event event;
        if (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                return false;
            }

            if (m_pKeyboard && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
            {
                IKeyboard::KeyCode code = ConvertToKeyCode(event.key.keysym.scancode);
                m_pKeyboard->SetKeyState(code, event.type == SDL_KEYDOWN);
            }

            if (m_pMouse)
            {
                if ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP))
                {
                    IMouse::Button button = ConvertToButton(event.button.button);
                    m_pMouse->SetButtonState(button, event.type == SDL_MOUSEBUTTONDOWN);
                }

                if (event.type == SDL_MOUSEWHEEL)
                {
                    m_pMouse->SetWheelX(event.wheel.x);
                    m_pMouse->SetWheelY(event.wheel.y);
                }

                if (event.type == SDL_MOUSEMOTION)
                {
                    m_pMouse->SetMousePosition(event.motion.x, event.motion.y);
                }
            }

            if (m_pController)
            {
                if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP)
                {
                    IGameController::PadButton button = ConvertToPadButton(event.cbutton.button, event);
                    m_pController->SetButtonState(button, event.type == SDL_CONTROLLERBUTTONDOWN);
                }

                // TODO:
                //   [] Support game controller trigger events

                //if (event.type == SDL_CONTROLLER_A)
                //{
                //    auto temp = event.cbutton.button;
                //    IGameController::PadButton button = ConvertToPadButton(event.jbutton.button, event);
                //    auto id = event.caxis.which;
                //    auto type = event.caxis.type;
                //    m_pController->SetButtonState(button, event.jbutton.state == m_pController->GetButtonState(button));
                //}

            }
        }

        return true;
    }
    virtual void NextFrame() override
    {
        if (m_pKeyboard)
            m_pKeyboard->NextFrame();

        if (m_pMouse)
            m_pMouse->NextFrame();

        if (m_pController)
            m_pController->NextFrame();
    }

    // ===== Keyboard =====
    virtual bool AttachKeyboard(std::unique_ptr<IKeyboard> pInput) override
    {
        if (pInput == nullptr)
            return false;
        m_pKeyboard = std::move(pInput);
        return true;
    }
    virtual IKeyboard* GetKeyboard() override
    {
        return m_pKeyboard.get();
    }

    // ===== Mouse =====
    virtual bool AttachMouse(std::unique_ptr<IMouse> pInput) override
    {
        if (pInput == nullptr)
            return true;

        m_pMouse = std::move(pInput);
        return true;
    }
    virtual IMouse* GetMouse() override
    {
        return m_pMouse.get();
    }

    virtual bool AttachController(std::unique_ptr<IGameController> pInput) override
    {
        if (pInput == nullptr)
            return false;

        m_pController = std::move(pInput);
        return true;
    }
    virtual IGameController* GetController() override
    {
        return m_pController.get();
    }

    IKeyboard::KeyCode ConvertToKeyCode(uint32_t key) const
    {
        if (m_keyMap.find(key) != m_keyMap.end())
        {
            return static_cast<IKeyboard::KeyCode>(m_keyMap.at(key));
        }

        return IKeyboard::KeyCode::kCount;
    }
    IMouse::Button ConvertToButton(uint32_t button) const
    {
        static constexpr uint32_t kSDLLeftButton = 1;
        static constexpr uint32_t kSDLWheelButton = 2;
        static constexpr uint32_t kSDLRightButton = 3;

        if (button == kSDLLeftButton)
        {
            return IMouse::kBtnLeft;
        }

        else if (button == kSDLRightButton)
        {
            return IMouse::kBtnRight;
        }

        else if (button == kSDLWheelButton)
        {
            return IMouse::kBtnWheelClicked;
        }

        return IMouse::kBtnMax;
    }
    IGameController::PadButton ConvertToPadButton(uint32_t button, SDL_Event event) const
    {
        switch (event.type)
        {
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        {
            if (button >= SDL_CONTROLLER_BUTTON_A && button <= SDL_CONTROLLER_BUTTON_Y)
            {
                return static_cast<IGameController::PadButton>(button - SDL_CONTROLLER_BUTTON_A + IGameController::kBtnA);
            }
        }
            break;
        case SDL_CONTROLLERAXISMOTION:
        {
            if (button >= SDL_CONTROLLER_AXIS_TRIGGERLEFT && button <= SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
            {
                return static_cast<IGameController::PadButton>(button - SDL_CONTROLLER_AXIS_TRIGGERLEFT + IGameController::kTriggerL);
            }
        }
            break;
        default:
            break;
        }

        return IGameController::kBtnMax;
    }
};

class WindowsSystem : public ISystem
{
public:
    virtual bool Initialize() override
    {
        if (!CheckSystemVersion())
            return false;

        if (m_pProcessorInfoReader = IProcessorInfoReader::Create();
            !m_pProcessorInfoReader->CheckProcessorStatus())
        {
            return false;
        }

        if (m_pMemoryInfoReader = IMemoryInfoReader::Create();
            !m_pMemoryInfoReader->CheckMemoryStatus())
        {
            return false;
        }

        if (m_pStorageInfoReader = IStorageInfoReader::Create();
            !m_pStorageInfoReader->CheckStorageStatus())
        {
            return false;
        }


        if(m_pGpuInfoReader = IGpuInfoReader::Create();
            !m_pGpuInfoReader->CheckGpuStatus())
        {
            return false;
        }


        //std::string str = GetCurrentTimeStr();
        //str = GetExecuteDirectory();
        //
        //std::cout << str << std::endl;
        return true;
    }

    virtual const char* GetSystemName() const override { return "Windows"; }

    virtual std::unique_ptr<IWindow> CreateSystemWindow(const char* pName, uint32_t width, uint32_t height) override
    {
        std::unique_ptr<IWindow> pWindow = std::make_unique<SDLWindow>();

        if (pWindow->Initialize(pName, width, height))
        {
            return pWindow;
        }
        return nullptr;
    }

    virtual std::vector<std::string> GetAllFiles(const std::string& path) override
    {
        std::vector<std::string> files;
        GetFilesRecursive(path, "", files);
        return files;
    }

    virtual void SetCurrentWorkingDirectory(const std::string& path) override
    {
        SetCurrentDirectoryA(path.c_str());
    }

    virtual void GetFilesRecursive(const std::string& path, const std::string& directory, std::vector<std::string>& files) override
    {
        std::string fullPath = path + "/" + directory + "/*";
        WIN32_FIND_DATAA findData;
        HANDLE findHandle = FindFirstFileA(fullPath.c_str(), &findData);
        if (findHandle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        std::string rootDirectory = directory;
        if (!rootDirectory.empty())
        {
            rootDirectory += "/";
        }

        while (true)
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::string directoryName = findData.cFileName;
                if (directoryName != "." && directoryName != "..")
                {
                    std::string nextDirectory = rootDirectory + directoryName;
                    GetFilesRecursive(path, nextDirectory, files);
                }
            }
            else
            {
                files.push_back(rootDirectory + findData.cFileName);
            }

            bool result = FindNextFileA(findHandle, &findData);
            if (!result)
            {
                break;
            }
        }
        FindClose(findHandle);
    }

    virtual bool CheckSystemVersion() override
    {
        //--------------------------------------------------------------
        // It checks registry and extract data from there.
        //--------------------------------------------------------------
        wchar_t productName[32] = { 0 };
        wchar_t currentBuildNumber[32] = { 0 };
        wchar_t releasedID[32] = { 0 };

        std::string osInfo;

        HKEY hKey;
        int i = 0;

        DWORD size = 128;

        // Try to access registry.
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        {
            LOG_INFO("Fail to Open OS information");
            return false;
        }

        // Get operation system's name
        if (RegQueryValueExW(hKey, L"ProductName", nullptr, nullptr, (LPBYTE)productName, &size) != ERROR_SUCCESS)
        {
            LOG_INFO("Fail to achieve product name");

            RegCloseKey(hKey);
            return false;
        }

        // Get OS's build number
        if (RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, (LPBYTE)currentBuildNumber, &size) != ERROR_SUCCESS)
        {
            LOG_INFO("Fail to achieve build number");
            RegCloseKey(hKey);
            return false;
        }

        // Get OS' release ID
        if (RegQueryValueExW(hKey, L"ReleaseId", nullptr, nullptr, (LPBYTE)releasedID, &size) != ERROR_SUCCESS)
        {
            LOG_INFO("Fail to achieve release id");
            RegCloseKey(hKey);
            return false;
        }

        // Bind to log text, which could be done in other class.
        //std::wstring ws(productName);
        std::string temp(WStrToStr(productName));
        std::string str;

        str = "OS Name\t\t:  " + temp + "\n";
        LOG_INFO(str.c_str());
        temp.clear();
        temp.assign(WStrToStr(currentBuildNumber));

        str = "Current Build\t:  " + temp + "\n";
        LOG_INFO(str.c_str());
        temp.clear();
        temp.assign(WStrToStr(releasedID));

        str = "Build Branch\t:  " + temp + "\n\n";
        LOG_INFO(str.c_str());
        temp.clear();

        RegCloseKey(hKey);

        return true;
    }

    //void ShowPopUpMessage(const char* pTitle, const char* pContext)
    virtual bool ShowPopUpMessage(const wchar_t* pTitle, const wchar_t* pContext) override
    {
        ///**************************************************************
        /// This is actually a simple pop-up messaging function for
        /// testing.
        ///**************************************************************
        if (MessageBoxW(nullptr, pTitle, pContext, MB_OK) == 3) // If aborted, return false
            return false;

        return true;
    }

    // Check file modification in run time
    virtual bool CheckFileModified(const char* pPath) override
    {
        DWORD notifyFilter = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

        constexpr DWORD kBufferSize = 1024 * 1024;
        BYTE* pBuffer = (PBYTE)malloc(kBufferSize);
        DWORD bytesReturned;
        FILE_NOTIFY_INFORMATION* pFileInfo;
        HANDLE hDir = CreateFileA
        (
            pPath, 
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            0
        );
        
        if (hDir == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        BOOL fileStatus = ReadDirectoryChangesW(hDir, pBuffer, kBufferSize, TRUE, notifyFilter, &bytesReturned, 0, 0);

        if (!fileStatus)
        {
            DWORD dwLastError = GetLastError();
            printf("error : %d\n", dwLastError);
            return false;
        }

        pFileInfo = (FILE_NOTIFY_INFORMATION*)(pBuffer);

        switch (pFileInfo->Action)
        {
        case FILE_ACTION_ADDED:
            LOG_INFO("FILE_ACTION_ADDED\n");
            return true;
            break;
        case FILE_ACTION_REMOVED:
            LOG_INFO("FILE_ACTION_REMOVED\n");
            return true;
        case FILE_ACTION_MODIFIED:
            LOG_INFO("FILE_ACTION_MODIFIED\n");
            return true;
        case FILE_ACTION_RENAMED_OLD_NAME:
            LOG_INFO("FILE_ACTION_RENAMED_OLD_NAME\n");
            return true;
        case FILE_ACTION_RENAMED_NEW_NAME:
            LOG_INFO("FILE_ACTION_RENAMED_NEW_NAME\n");
            return true;
        default:
            LOG_INFO("Nothing has chaged\n");
            return false;
            break;
        }

        return false;
    }
   
    virtual void ReadTextFile(const std::string& path) override
    {
        std::fstream file;
        file.open(path, std::ios_base::in);

        std::string context((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

        LOG(std::string("Priting context from: " + path + "\n"), false);
        LOG(context.data());

        file.close();
    }

    void StartNewProcess(const char* pPath) 
    {
        ///***************************************************************************
        /// This function takes a path to an external program
        /// Then the
        ///***************************************************************************

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        size_t newSize = strlen(pPath) + 1;

        wchar_t* wcstring = new wchar_t[newSize];

        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wcstring, newSize, pPath, _TRUNCATE);

        // Start the child process. 
        if (
            !CreateProcess
            (
                (LPCSTR)wcstring,                   // Module name (No - use command line)
                nullptr,                    // Command line
                nullptr,                    // Process handle not inheritable
                nullptr,                    // Thread handle not inheritable
                FALSE,                      // Set handle inheritance to FALSE
                0,                          // No creation flags
                nullptr,                    // Use parent's environment block
                nullptr,                    // Use parent's starting directory 
                &si,                        // Pointer to STARTUPINFO structure
                &pi)                        // Pointer to PROCESS_INFORMATION structure
            )
        {
            LOG_FATAL("StartExecutable failed", false);
            LOG_FATAL(GetLastErrorStdStr().c_str());

            return;
        }

        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    };
    
private:
    std::string GetLastErrorStdStr()
    {
        DWORD error = GetLastError();
        if (error)
        {
            LPVOID lpMsgBuf;
            DWORD bufLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)& lpMsgBuf,
                0, NULL);
            if (bufLen)
            {
                LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
                std::string result(lpMsgStr, lpMsgStr + bufLen);

                LocalFree(lpMsgBuf);

                return result;
            }
        }
        return std::string();
    }

    std::string GetExecuteDirectory()
    {
        char buffer[256];
        GetCurrentDirectoryA(256, buffer);
        return std::string(buffer);
    }
};

std::unique_ptr<ISystem> ISystem::Create()
{
#ifdef _WIN32
    return std::make_unique<WindowsSystem>();
#else
    return nullptr;
#endif
}

const std::string ISystem::GetCurrentTimeStr() const
{
    const std::time_t time = std::time(nullptr);

    std::tm buf;
    localtime_s(&buf, &time);
    std::stringstream wss;
    wss << std::put_time(&buf, "%X");

    return wss.str();
}