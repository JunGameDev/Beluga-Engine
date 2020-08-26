#pragma once
#include <Core/Layers/ApplicationLayer.h>
#include <Core/Camera/Camera.h>

class TestLogic : public Bel::IGameLayer
{
public:
    TestLogic(size_t screenWidth, size_t screenHeight)
        : IGameLayer(0, 0)
    {
        m_camera = Bel::Camera2D(static_cast<uint32_t>(screenWidth), static_cast<uint32_t>(screenHeight));
    }
    virtual ~TestLogic() {}
    virtual const char* GetGameName() const override { return "Project_Mario"; }
    virtual void LoadLevel(Bel::IEvent* pEvent) override
    {
    }

    virtual bool Initialize() override
    {
        return true;

    }
    virtual void RegisterWithLua() override
    {

    }
};

class TestApp : public Bel::ApplicationLayer
{
public:
    TestApp() {}
    virtual ~TestApp() {}
    virtual std::unique_ptr<Bel::IGameLayer> CreateGameLayer(size_t width, size_t height) override
    {
        return std::make_unique<TestLogic>(width, height);
    }

    virtual bool Initialize() override
    {
        if (!ApplicationLayer::Initialize())
            return false;

        return true;
    }
};
