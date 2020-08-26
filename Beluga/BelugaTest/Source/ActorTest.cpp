#include <Actors/Actor.h>
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

class TestComponent : public Bel::IActorComponent
{
public:
    TestComponent(Bel::Actor* pOwner, const char* pName)
        : Bel::IActorComponent(pOwner, pName)
    {
    }

    virtual ~TestComponent() {}
    virtual bool Initialize(tinyxml2::XMLElement* pData) override { return true; }
};

static std::unique_ptr<Bel::IActorComponent> CreateTestComponent(Bel::Actor* pOwner, const char* pName)
{
    return std::unique_ptr<Bel::IActorComponent>(new TestComponent(pOwner, pName));
}

namespace BelugaTest
{
    TEST_CLASS(ActorFactoryTest)
    {
    public:
        TEST_METHOD(GetNextActorId)
        {
            ActorFactory actorFactory;

            for (size_t i = 0; i < 100000; ++i)
            {
                Assert::AreEqual(static_cast<uint32_t>(i), actorFactory.GetNextActorId());
            }
        }

        TEST_METHOD(RegisterComponentCreateor)
        {
            ActorFactory actorFactory;
            actorFactory.RegisterComponentCreator("TestComponent", &CreateTestComponent);
            Assert::AreEqual(static_cast<size_t>(1), actorFactory.GetNumComponentCreator());
        }

        TEST_METHOD(CreateEmptyActor)
        {
            ActorFactory actorFactory;
            auto pActor = actorFactory.CreateActorWithEmpty();
            Assert::IsNotNull(pActor.get());
        }

        TEST_METHOD(CreateActorByFileName)
        {
            ActorFactory actorFactory;
            actorFactory.RegisterComponentCreator("TestComponent", &CreateTestComponent);
            Assert::AreEqual(static_cast<size_t>(1), actorFactory.GetNumComponentCreator());

            auto pActor = actorFactory.CreateActorByFileName("../../BelugaTest/Assets/CreateActorByFileName/TestObject.xml");
            Assert::IsNotNull(pActor.get());

            Assert::IsTrue(pActor->HasComponent(IActorComponent::HashName("TestComponent")));

            TestComponent* pTestComponent = static_cast<TestComponent*>(pActor->GetComponent(IActorComponent::HashName("TestComponent")));
            Assert::IsNotNull(pTestComponent);
        }
    };
}
