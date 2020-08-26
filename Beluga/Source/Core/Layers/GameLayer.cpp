#include "Core/Layers/GameLayer.h"
#include "Core/Layers/ApplicationLayer.h"

using namespace Bel;

static std::unique_ptr<IActorComponent> CreateTransformComponent(Actor* pOwner, const char* pName)
{
    return std::unique_ptr<IActorComponent>(new TransformComponent(pOwner, pName));
}

static std::unique_ptr<IActorComponent> CreateStaticBodyComponent(Actor* pOwner, const char* pName)
{
    return std::unique_ptr<IActorComponent>(ApplicationLayer::GetInstance()->GetGameLayer()->GetPhysicsManager().CreateStaticBodyComponent(pOwner, pName));
}

static std::unique_ptr<IActorComponent> CreateDynamicBodyComponent(Actor* pOwner, const char* pName)
{
    return std::unique_ptr<IActorComponent>(ApplicationLayer::GetInstance()->GetGameLayer()->GetPhysicsManager().CreateDynamicBodyComponent(pOwner, pName));
}

IGameLayer::IGameLayer(float&& xGravity, float&& yGravity)
    : m_xGravity(xGravity)
    , m_yGravity(yGravity)
{
    m_actorFactory.RegisterComponentCreator("StaticBodyComponent", &CreateStaticBodyComponent);
    m_actorFactory.RegisterComponentCreator("DynamicBodyComponent", &CreateDynamicBodyComponent);
    m_actorFactory.RegisterComponentCreator("TransformComponent", &CreateTransformComponent);
}
