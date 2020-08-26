#include <iostream>
#include <memory>

#include "Core/Layers/ApplicationLayer.h"
#include "Systems/System.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Parshing/tinyxml2.h"
#include "Input/Input.h"

using namespace tinyxml2;
using namespace Bel;

using SeverityLevel = Logging::SeverityLevel;

namespace Lua
{
    static int GetMouseX(lua_State* pState)
    {
        IMouse* pMouse = ApplicationLayer::GetInstance()->GetMouseInput();
        lua_pushnumber(pState, pMouse->GetMouseX());
        return 1;
    }

    static int GetMouseY(lua_State* pState)
    {
        IMouse* pMouse = ApplicationLayer::GetInstance()->GetMouseInput();
        lua_pushnumber(pState, pMouse->GetMouseY());
        return 1;
    }
}


ApplicationLayer* ApplicationLayer::s_pAppLayer = nullptr;
float ApplicationLayer::s_delta = 0;

bool ApplicationLayer::Initialize()
{
    s_pAppLayer = this;

    if (!m_logging.Initialize())
    {
        return false;
    }
    m_logging.Log(SeverityLevel::kLevelDebug, "Initialize");

    // --- System ---
    {
        m_pSystem = ISystem::Create();
        if (m_pSystem == nullptr)
        {
            return false;
        }

        if (!m_pSystem->Initialize())
        {
            LOG_FATAL("Failed to initialize system");
            return false;
        }

        m_logging.Log(SeverityLevel::kLevelDebug, "System: ", false);
        m_logging.Log(SeverityLevel::kLevelDebug, m_pSystem->GetSystemName());
    }

    // --- Audio ---
    {
        m_pAudio = IAudio::Create();
        if (m_pAudio == nullptr)
        {
            return false;
        }
        if (!m_pAudio->Initialize())
        {
            return false;
        }
    }
 
    size_t width = (m_configs.find("Width") != m_configs.end()) ? std::stoi(m_configs["Width"]) : 1000;
    size_t height = (m_configs.find("Height") != m_configs.end()) ? std::stoi(m_configs["Height"]) : 1000;

    // --- Game Layer ---
    {
        m_pGameLayer = CreateGameLayer(width, height);
        if (m_pGameLayer == nullptr)
        {
            return false;
        }

        m_logging.Log(SeverityLevel::kLevelDebug, "Game: ", false);
        m_logging.Log(SeverityLevel::kLevelDebug, m_pGameLayer->GetGameName());
    }

    // --- Window ---
    {
        m_pWindow = m_pSystem->CreateSystemWindow(m_pGameLayer->GetGameName(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        if (m_pWindow == nullptr)
        {
            return false;
        }
    }

    // --- Input ---
    {
        // --- Keyboard ---
        auto pKeyboard = IKeyboard::Create();
        if (pKeyboard == nullptr)
        {
            return false;
        }
        if (!pKeyboard->Initialize())
        {
            return false;
        }

        // --- Mouse ---
        auto pMouse = IMouse::Create();
        if (pMouse == nullptr)
        {
            return false;
        }
        if (!pMouse->Initialize())
        {
            return false;
        }

        // --- Game Controller ---
        auto pController = IGameController::Create();
        if (pController == nullptr)
        {
            LOG_FATAL("Unable to create game controller");
            // return false;
        }
        if (!pController->Initialize())
        {
            LOG_FATAL("Unable to initialize controller");
            //return false;
        }

        m_pWindow->AttachKeyboard(std::move(pKeyboard));
        m_pWindow->AttachMouse(std::move(pMouse));
        m_pWindow->AttachController(std::move(pController));
    }

    // --- Graphics ---
    {
        m_pGraphics = IGraphics::Create();

        if (m_pGraphics == nullptr)
        {
            return false;
        }

        if (!m_pGraphics->Initialize(m_pWindow.get()))
        {
            return false;
        }
    }

    if (!m_pGameLayer->Initialize())
    {
        LOG_FATAL("Failed to initialize game logic");
        return false;
    }

    return true;
}

void ApplicationLayer::Run()
{
    LOG("Run");

    auto previouseTime = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = time - previouseTime;

        if (!m_pWindow->ProcessEvents())
        {
            return;
        }

        s_delta = delta.count();
        m_pGameLayer->Update(s_delta);
        m_pWindow->NextFrame();
        previouseTime = time;
    }
}

void ApplicationLayer::Shutdown()
{
    m_pSystem = nullptr; // Automatically frees memory
}

bool ApplicationLayer::LoadConfig(std::string fileName)
{
    fileName += ".xml";

    tinyxml2::XMLDocument doc;
    doc.LoadFile(fileName.c_str());

    XMLElement* pRoot = doc.RootElement();

    if (!pRoot)
    {
        m_logging.Log(SeverityLevel::kLevelError, "Failed to load a config file");
        return false;
    }

    // Loop through each child element and load the component
    for (XMLElement* pElem = pRoot->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
    {
        for (XMLElement* pConfigElem = pElem->FirstChildElement(); pConfigElem; pConfigElem = pConfigElem->NextSiblingElement())
        {
            const char* pKey = pConfigElem->Attribute("id");
            const char* pValue = pConfigElem->Attribute("value");

            // If both key and text exist
            if (pKey && pValue)
            {
                // Then add it to m_configs
                m_configs.emplace(pKey, pValue);
            }
        }
    }

    return true;
}

void ApplicationLayer::RegisterWithLua()
{
    auto& scripting = m_pGameLayer->GetScriptingManager();
    scripting.GetGlobal("g_logic");

    scripting.CreateTable();
    scripting.AddToTable("GetX", Lua::GetMouseX);
    scripting.AddToTable("GetY", Lua::GetMouseY);
    scripting.AddToTable("Mouse");

    scripting.PopAll();
}
