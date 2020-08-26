#include "CppUnitTest.h"
#include "Audio/Audio.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

namespace BelugaTest
{
    TEST_CLASS(AudioTest)
    {
    public:
        TEST_METHOD(Create)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
        }

        TEST_METHOD(Initialize)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
            Assert::IsTrue(pAudio->Initialize());
        }

        TEST_METHOD(PlayMusic)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
            Assert::IsTrue(pAudio->Initialize());

            Assert::IsTrue(pAudio->PlayMusic("../../BelugaTest/Assets/PlayMusic/TestMusic_1.mp3"));
        }

        TEST_METHOD(PlaySFX)
        {
            //auto pAudio = IAudio::Create();
            //Assert::IsNotNull(pAudio.get());
            //Assert::IsTrue(pAudio->Initialize());

            //Assert::IsTrue(pAudio->PlaySoundEffect("../../BelugaTest/Assets/PlaySFX/TestSound.wav"));
        }

        TEST_METHOD(ChangeMusic)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
            Assert::IsTrue(pAudio->Initialize());

            Assert::IsTrue(pAudio->PlayMusic("../../BelugaTest/Assets/PlayMusic/TestMusic_1.mp3"));
            Assert::IsTrue(pAudio->ChangeMusic("../../BelugaTest/Assets/PlayMusic/TestMusic_2.mp3", 10));
        }

        TEST_METHOD(FadeInMusic)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
            Assert::IsTrue(pAudio->Initialize());
            Assert::IsTrue(pAudio->PlayMusic("../../BelugaTest/Assets/PlayMusic/TestMusic_1.mp3"));
            Assert::IsTrue(pAudio->FadeInMusic(20));
        }

        TEST_METHOD(FadeOutMusic)
        {
            auto pAudio = IAudio::Create();
            Assert::IsNotNull(pAudio.get());
            Assert::IsTrue(pAudio->Initialize());
            Assert::IsTrue(pAudio->PlayMusic("../../BelugaTest/Assets/PlayMusic/TestMusic_1.mp3"));
            Assert::IsTrue(pAudio->FadeOutMusic(20));
        }
    };
}