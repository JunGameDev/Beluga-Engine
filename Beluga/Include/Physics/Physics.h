// Copyright (c) 2020 Junyoung Kim

#pragma once

#include <memory>
#include "Graphics/Graphics.h"
#include "Actors/Actor.h"
#include "Core/Math/Vector2.h"
#include "Scripting/Scripting.h"
#include "Physics/PhysicsShape.h"

static const Bel::IActorComponent::Id kTransformId = Bel::IActorComponent::HashName("TransformComponent");
static const Bel::IActorComponent::Id kDynamicBodyId = Bel::IActorComponent::HashName("DynamicBodyComponent");
static const Bel::IActorComponent::Id kStaticBodyId = Bel::IActorComponent::HashName("StaticBodyComponent");

namespace Bel
{
    struct PointFloat;

    class TransformComponent : public IActorComponent
    {
    private:
        Vector2<float> m_position;
        float   m_degree;
        float   m_radian;
        int     m_speed;

    public:
        TransformComponent(Actor* pOwner, const char* pName)
            : IActorComponent(pOwner, pName)
            , m_position(0.0f, 0.0f)
            , m_degree(0.f)
            , m_radian(0.f)
            , m_speed(1)
        {
        }
        virtual ~TransformComponent() {}

        virtual bool Initialize(tinyxml2::XMLElement* pData) override;

        LUA_REGISTER();

        void Move(const float& x, const float& y)
        {
            m_position.m_x += x * m_speed;
            m_position.m_y += y * m_speed;
        }

        void MoveConstantly(const float& x, const float& y)
        {
            m_position.m_x += x;
            m_position.m_y += y;
        }

        void SetPosition(const float& x, const float& y)
        {
            m_position.m_x = x;
            m_position.m_y = y;
        }

        Vector2<float>& GetPosition() { return m_position; }

        float GetDegree() const { return m_degree; }
        void SetDegree(float degree);
        void SetRadian(float radian);

        const int GetSpeed() { return m_speed; }
        void SetSpeed(int speed) { m_speed = speed; }

    };

    class IStaticBodyComponent : public IActorComponent
    {
    public:
        IStaticBodyComponent(Actor* pOwner, std::string_view name)
            : IActorComponent(pOwner, name)
        {
        }
        virtual ~IStaticBodyComponent() {}
        virtual void SetDimensions(float width, float height) = 0;
        virtual void SetPosition(float x, float y) = 0;
    };

    class IDynamicBodyComponent : public IActorComponent
    {
    public:
        IDynamicBodyComponent(Actor* pOwner, std::string_view name)
            : IActorComponent(pOwner, name)
        {
        }
        virtual ~IDynamicBodyComponent() {}
        virtual void SetLinearVelocity(Vector2<float> vec) = 0;
        virtual void SetLinearVelocity(float x, float y) = 0;
        virtual void SetLinearVelocityX(float value) = 0;
        virtual void SetLinearVelocityY(float value) = 0;
        virtual Vector2<float> GetLinearVelocity() = 0;
        virtual float GetAngularVelocity() = 0;
        virtual void ApplyForce(float x, float y) = 0;
        virtual void ApplyForceToCenter(float x, float y, bool awake) = 0;
        virtual void ApplyLinearImpulse(float x, float y) = 0;
        virtual void ApplyTorque(float val) = 0;
        virtual void SetPosition(float x, float y) = 0;
        virtual void SetRadian(float rad) = 0;
        virtual float GetMass() = 0;
        virtual void ReDimension(float width, float height, const char* pUserData, Vector2<float> center) = 0;
    };

    struct Collision
    {
        Actor* m_pActor;
        IActorComponent* m_pCollider;
        std::string m_userData;
    };

    struct CollisionGroup
    {
        Collision m_colA;
        Collision m_colB;
    };

    class ContactListener
    {
    public:
        virtual void BeginActorCollision(CollisionGroup& group);
        virtual void EndActorCollision(CollisionGroup& group);
        virtual void BeginActorTrigger(CollisionGroup& group);
        virtual void EndActorTrigger(CollisionGroup& group);
    };

    class IPhysicsManager
    {
        static constexpr float kPixelPerMeter = 100;
    public:
        virtual ~IPhysicsManager() {};
        virtual bool Initialize() = 0;
        virtual void Update(float delta) = 0;
        virtual void DrawDebugLine() = 0;

        virtual void RegisterStaticBody(IStaticBodyComponent* pComponent) = 0;
        virtual void UnregisterStaticBody(IStaticBodyComponent* pComponent) = 0;
        virtual void RegisterDynamicBody(IDynamicBodyComponent* pComponent) = 0;
        virtual void UnregisterDynamicBody(IDynamicBodyComponent* pComponent) = 0;

        virtual float GetGravity() = 0;

        virtual void SetContactListener(std::shared_ptr<ContactListener> pContactListener) = 0;

        virtual std::unique_ptr<IStaticBodyComponent> CreateStaticBodyComponent(Actor* pOwner, const char* pName) = 0;
        virtual std::unique_ptr<IDynamicBodyComponent> CreateDynamicBodyComponent(Actor* pOwner, const char* pName) = 0;
        static std::unique_ptr<IPhysicsManager> Create(float xGravity, float yGravity);

        static constexpr float GetPPM() { return kPixelPerMeter; }
        static constexpr float ConvertMeterToPixel(float meter) { return meter * kPixelPerMeter; }
    };
}
