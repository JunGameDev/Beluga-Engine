#pragma once

#include <memory>
#include <Core/Layers/ApplicationLayer.h>
#include <Events/Events.h>
#include <Resources/Resource.h>
#include <Physics/Physics.h>

class GameLogic : public Bel::IGameLayer
{
public:
    GameLogic(size_t screenWidth, size_t screenHeight);
    virtual ~GameLogic() {}

    // Inherited via IGameLayer
    virtual const char* GetGameName() const override { return "SandBox"; }
    virtual bool Initialize() override;
    virtual void LoadLevel(Bel::IEvent* pEvent) override;
};

class GameApp : public Bel::ApplicationLayer
{
public:
    GameApp() {}
    virtual ~GameApp() {}
    virtual std::unique_ptr<Bel::IGameLayer> CreateGameLayer(size_t width, size_t height) override;

    virtual bool Initialize() override;
};