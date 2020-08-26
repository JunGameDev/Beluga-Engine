#pragma once

#include <unordered_map>
#include "Systems/System.h"
#include "GameLayer.h"
#include "Log/Logging.h"
#include "Graphics/Graphics.h"
#include "Audio/Audio.h"

namespace Bel
{
    /// Class Description
    /// 
    /// Application layer is the house for the code for all API specific call.
    /// Any application powered by Beluga-Engine must use application layer as its entry point.
    /// Note that this class is singleton.
    ///
    /// Currently, it is handling following features.
    /// - File operation
    /// - Input device handling 
    /// - Initializing, and shutting down the game
    /// - The main game loop
    /// - Rendering
    class ApplicationLayer
    {
    public:
        using ConfigMap = std::unordered_map<std::string, std::string>;
        
    protected:
        Logging m_logging;

        std::unique_ptr<ISystem> m_pSystem;
        std::unique_ptr<IWindow> m_pWindow;

        std::unique_ptr<IGraphics> m_pGraphics;
        std::unique_ptr<IAudio> m_pAudio;

        std::unique_ptr<IGameLayer> m_pGameLayer;
        
        // A map to hold key-value pair for initial engine configuration.
        ConfigMap m_configs;

    private:
        static ApplicationLayer* s_pAppLayer;
        static float s_delta;

    public:
        virtual ~ApplicationLayer() {}

        static ApplicationLayer* GetInstance() 
        { 
            return s_pAppLayer;
        }
        static float GetDeltaTime()
        {
            return s_delta;
        }

        virtual std::unique_ptr<IGameLayer> CreateGameLayer(size_t width, size_t height) = 0;

        // ===== Basic core =====
        virtual bool Initialize();
        void Run();
        void Shutdown();

        // ===== Logging =====
        Logging& GetLogging() { return m_logging; }

        // ===== Engine Configuration =====
        const ConfigMap GetConfiguration() const { return m_configs; }
        bool LoadConfig(std::string fileName);

        // ===== Scripting =====
        void RegisterWithLua();

        // ===== Getters =====
        IGraphics*       GetGraphics()        const { return m_pGraphics.get();          }
        IKeyboard*       GetKeyboardInput()   const { return m_pWindow->GetKeyboard();   }
        IMouse*          GetMouseInput()      const { return m_pWindow->GetMouse();      }
        IGameController* GetControllerInput() const { return m_pWindow->GetController(); }
        IGameLayer*      GetGameLayer()       const { return m_pGameLayer.get();         }
        IAudio*          GetAudio()           const { return m_pAudio.get();             }
    };

    // Core log macros
    #define LOG(...)          ::Bel::ApplicationLayer::GetInstance()->GetLogging().Log(Logging::SeverityLevel::kLevelDebug, __VA_ARGS__)
    #define LOG_INFO(...)     ::Bel::ApplicationLayer::GetInstance()->GetLogging().Log(Logging::SeverityLevel::kLevelInfo, __VA_ARGS__)
    #define LOG_WARNING(...)  ::Bel::ApplicationLayer::GetInstance()->GetLogging().Log(Logging::SeverityLevel::kLevelWarn, __VA_ARGS__)
    #define LOG_ERROR(...)    ::Bel::ApplicationLayer::GetInstance()->GetLogging().Log(Logging::SeverityLevel::kLevelInfo, __VA_ARGS__)
    #define LOG_FATAL(...)    ::Bel::ApplicationLayer::GetInstance()->GetLogging().Log(Logging::SeverityLevel::kLevelFatal, __VA_ARGS__)

    #define Camera            ::Bel::ApplicationLayer::GetInstance()->GetGameLayer()->GetCamera()
    #define DeltaTime         ::Bel::ApplicationLayer::GetInstance()->GetDeltaTime()
}
