#include "CppUnitTest.h"
#include <time.h>
#include <Core/Math/Vector2.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

namespace BelugaTest
{
    TEST_CLASS(Vector2Test)
    {
    public:
        TEST_METHOD(IsIntializedToZero)
        {
            Vector2<float> vec;
            Assert::AreEqual(vec.m_x, 0.f);
            Assert::AreEqual(vec.m_y, 0.f);
        }

        TEST_METHOD(IsStaticZeroValueZero)
        {
            Assert::AreEqual(Vector2<float>::s_Zero.m_x, 0.f);
            Assert::AreEqual(Vector2<float>::s_Zero.m_y, 0.f);
        }

        TEST_METHOD(IsZero)
        {
            Vector2<float> vecF(0, 0);
            Assert::IsTrue(vecF.IsZero());
            vecF = Vector2<float>(1.f, 1.f);
            Assert::IsFalse(vecF.IsZero());
            
            Vector2<int> vecInt(0, 0);
            Assert::IsTrue(vecInt.IsZero());
            vecInt = Vector2<int>(1, 1);
            Assert::IsFalse(vecInt.IsZero());

            Vector2<double> vecD(0, 0);
            Assert::IsTrue(vecD.IsZero());
            vecD = Vector2<double>(1, 1);
            Assert::IsFalse(vecD.IsZero());
        }

        TEST_METHOD(Setter)
        {
            srand(static_cast<unsigned int>(time(nullptr)));

            Vector2<float> vec(0, 0);
            Assert::IsTrue(vec.IsZero());

            for (size_t i = 0; i < 100000; ++i)
            {
                float x = static_cast<float>(rand() % 10000);
                float y = static_cast<float>(rand() % 10000);

                vec.Set(x, y);
                Assert::IsTrue(vec == Vector2<float>(x, y));
            }
        }

        TEST_METHOD(LengthSquared)
        {

        }
    };

}
