#include "CppUnitTest.h"
#include <Input/Input.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

namespace BelugaTest
{
    TEST_CLASS(InputTest)
    {
    public:
        TEST_METHOD(Create)
        {
            auto pkeyboard = CreateKeyboard();
            auto pMouse = CreateMouse();
        }

        TEST_METHOD(KeyboardPressedNotNull)
        {
            auto pkeyboard = CreateKeyboard();
        }
    private:
        std::unique_ptr<IKeyboard> CreateKeyboard()
        {
            auto pKeyboard = IKeyboard::Create();
            Assert::IsNotNull(pKeyboard.get());
            return std::move(pKeyboard);
        }

        std::unique_ptr<IMouse> CreateMouse()
        {
            auto pMouse = IMouse::Create();
            Assert::IsNotNull(pMouse.get());
            return std::move(pMouse);
        }
    };
}