#include "CppUnitTest.h"
#include <Systems/System.h>
#include <Graphics/Graphics.h>
#include <Log/Logging.h>
#include "TestApp.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

namespace BelugaTest
{
    TEST_CLASS(GraphicsUnitTest)
    {
    public:
        TEST_METHOD(Create)
        {
            auto pGraphics = CreateGraphics();
        }

        TEST_METHOD(Initialize)
        {
            auto pGraphics = CreateAndInitialize();
        }

        TEST_METHOD(CreateTexture)
        {
            auto pGraphics = CreateAndInitialize();
            auto pTexture = pGraphics->LoadTextureDirectly("../../BelugaTest/Assets/CreateTexture/test.png");
            Assert::IsNotNull(pTexture.get());
        }

        TEST_METHOD(RenderRect)
        {
            auto pGraphics = CreateAndInitialize();
            Rect testRect = { 0, 0, 50, 50 };
            Color testColor = { 255, 255, 255, 255 };
            Assert::IsTrue(pGraphics->RenderRect(testRect, testColor));
        }

        TEST_METHOD(RenderFillRect)
        {
            auto pGraphics = CreateAndInitialize();
            Rect testRect = { 0, 0, 50, 50 };
            Color testColor = { 255, 255, 255, 255 };
            Assert::IsTrue(pGraphics->RenderFillRect(testRect, testColor));
        }

        TEST_METHOD(RenderLine)
        {
            auto pGraphics = CreateAndInitialize();
            Point testSrc = { 0, 0 };
            Point testDest = { 10, 10 };
            Color testColor = { 255, 255, 255, 255 };

            Assert::AreEqual(pGraphics->RenderLine(testSrc, testDest, testColor), static_cast<bool>(1));
            Assert::IsTrue(pGraphics->RenderLine(testSrc, testDest, testColor));
        }

        TEST_METHOD(DrawTexture)
        {
            auto pGraphics = CreateAndInitialize();
            auto pTexture = pGraphics->LoadTextureDirectly("../../BelugaTest/Assets/CreateTexture/test.png");
            Assert::IsNotNull(pTexture.get());
            Rect testRect = { 0, 0, 181, 174 };
            TestApp app;
            Assert::IsTrue(app.Initialize());
            auto pLogic = app.CreateGameLayer(100, 100);
            Assert::IsNotNull(pLogic.get());
            Assert::IsTrue(pLogic->Initialize());
            Assert::IsTrue(pGraphics->DrawTexture(pTexture.get()));
        }

    private:
        std::unique_ptr<IGraphics> CreateGraphics()
        {
            auto pGraphics = IGraphics::Create();
            Assert::IsNotNull(pGraphics.get());
            
            return std::move(pGraphics);
        }

        std::unique_ptr<IGraphics> Initialize(std::unique_ptr<IGraphics> pGraphics)
        {
            auto pSystem = ISystem::Create();
            Assert::IsNotNull(pSystem.get());

            auto pWindow = pSystem->CreateSystemWindow("UnitTest", 640, 480);
            Assert::IsNotNull(pWindow.get());
            Assert::IsTrue(pGraphics->Initialize(pWindow.get()));

            return std::move(pGraphics);
        }

        std::unique_ptr<IGraphics> CreateAndInitialize()
        {
            return std::move(Initialize(CreateGraphics()));
        }
    };
}

