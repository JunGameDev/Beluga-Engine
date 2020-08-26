#include "Game.h"

using namespace Bel;

/****************************************************************
 *                      Game Logic
 ***************************************************************/
GameLogic::GameLogic(size_t screenWidth, size_t screenHeight)
    : IGameLayer(0, -10.0f)
{
    ApplicationLayer::GetInstance()->GetLogging().AddCategory(Logging::SeverityLevel::kLevelFatal, "GameLogic");
    m_camera = Camera2D(static_cast<uint32_t>(screenWidth), static_cast<uint32_t>(screenHeight));
}

bool GameLogic::Initialize()
{
    bool result = IGameLayer::Initialize();

    if (!result)
    {
        return result;
    }

    return true;
}

void GameLogic::LoadLevel(IEvent* pEvent)
{
}

/****************************************************************
 *                      Game App
 ***************************************************************/
std::unique_ptr<IGameLayer> GameApp::CreateGameLayer(size_t width, size_t height)
{
    return std::make_unique<GameLogic>(width, height);
}

bool GameApp::Initialize()
{
    LoadConfig("../Configuration");

    if (!ApplicationLayer::Initialize())
        return false;

    m_logging.SetConfiguration(m_configs);

    return true;
}