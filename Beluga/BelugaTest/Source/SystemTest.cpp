#include "CppUnitTest.h"
#include <memory>
#include <Core/Layers/ApplicationLayer.h>
#include <Systems/System.h>
#include <Input/Input.h>
#include <Log/Logging.h>
#include <memory>
#include "TestApp.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

std::unique_ptr<ISystem> CreateSystem()
{
    auto pSystem = ISystem::Create();
    Assert::IsNotNull(pSystem.get());

    return std::move(pSystem);
}

namespace BelugaTest
{
    TEST_CLASS(SystemTest)
    {
    public:
        TEST_METHOD(Create)
        {
            auto pSystem = CreateSystem();
        }

        TEST_METHOD(GetSystemName)
        {
            auto pSystem = CreateSystem();
            Assert::AreEqual("Windows", pSystem->GetSystemName());
        }

        TEST_METHOD(GetAllFiles)
        {
            auto pSystem = CreateSystem();

            auto files = pSystem->GetAllFiles("../../BelugaTest/Assets/GetAllFiles");
            Assert::AreEqual(static_cast<size_t>(1), files.size());
            Assert::AreEqual("test.txt", files[0].c_str());
        }

        TEST_METHOD(CheckProcessor)
        {
            TestApp app;
            Assert::IsTrue(app.Initialize());

            auto pSystem = CreateSystem();
            Assert::AreEqual("Windows", pSystem->GetSystemName());
           // Assert::IsTrue(pSystem->CheckProcessor());
        }

        //TEST_METHOD(ShowPopUpMessage)
        //{
        //    auto pSystem = CreateSystem();
        //    Assert::AreEqual("Windows", pSystem->GetSystemName());

        //    Assert::IsTrue(pSystem->ShowPopUpMessage(L"Test Pop Up Message", L"This is test message"));
        //}
    };

}
