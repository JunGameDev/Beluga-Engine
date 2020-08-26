#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string_view>

#include "Scripting/Scripting.h"
#include "Parshing/tinyxml2.h"

namespace Bel
{
    class Actor;
    class IGraphics;
    class IView;
    class ResourceHandle;
    class ScriptingManager;

    // Empty struct for identifying  each component.
    // This is required for checking which kind of family type that component came from,
    // when GetComponent<T> or HasComponent<T> in Actor class being called.
    // Currently not being used.
    struct ComponentIdentifier {};

    class IActorComponent
    {
    public:
        typedef uint32_t Id;

    private:
        Actor* m_pOwner;
        Id m_familyId;
        Id m_compId;

    public:
        IActorComponent(Actor* pOwner, std::string_view name)
            : m_pOwner(pOwner)
            , m_compId(HashName(name))
            , m_familyId(HashName(name))
        {
        }
        virtual ~IActorComponent() {}

        virtual bool Initialize(tinyxml2::XMLElement* pData) = 0;
        virtual bool PostInit() { return true; }
        virtual void Update(float delta) {}
        virtual void Render(IGraphics* pGraphics) {}

        virtual void RegisterWithScript() {}

        Actor* GetOwner() { return m_pOwner; }

        Id GetId()         const { return m_compId; }
        Id GetFamilyId()   const { return m_familyId; }


        static Id HashName(std::string_view name)
        {
            std::hash<std::string> hasher;
            return static_cast<Id>(hasher(std::string(name)));
        }
    };

    class Actor
    {
    public:
        typedef uint32_t Id;

    private:
        Id m_id;
        //IView* m_pOwnerView;

        std::string m_name;
        std::unordered_map<IActorComponent::Id, std::unique_ptr<IActorComponent>> m_components;

    public:
        Actor(Id id)
            : m_id(id)
        {
        }

        ~Actor()
        {
            m_components.clear();
        }

        bool Initialize(tinyxml2::XMLElement* pData);
        bool PostInit();
        void Destroy();
        void Update(float delta);
        void Render(IGraphics* pGraphics);

        void AddComponent(std::unique_ptr<IActorComponent> pComponent);
        IActorComponent* GetComponent(IActorComponent::Id id);
        bool HasComponent(IActorComponent::Id id);

        std::unordered_map<IActorComponent::Id, std::unique_ptr<IActorComponent>>* GetComponents() { return &m_components; }

        Id GetId() const { return m_id; }
        void SetName(const std::string_view& name) { m_name = name; }
        
        const std::string& GetName() const { return m_name; }
        bool IsName(const char* pText);

        void RegisterWithScript();
    };

    class ActorFactory
    {
    public:
        typedef std::function<std::unique_ptr<IActorComponent>(Actor*, const char*)> ComponentFunction;
    
    private:
        Actor::Id m_nextActorId;
        std::unordered_map<std::string, ComponentFunction> m_actorComponentCreatorMap;

    public:
        ActorFactory()
            : m_nextActorId(0)
        {
        }
        ~ActorFactory() {}

        std::shared_ptr<Actor> CreateActorWithEmpty();
        std::shared_ptr<Actor> CreateActorByFileName(const std::string_view& fileName);
        std::shared_ptr<Actor> CreateActorByResource(std::shared_ptr<ResourceHandle> pResource);

        void RegisterComponentCreator(const char* pComponentName, ComponentFunction pFunction)
        {
            m_actorComponentCreatorMap[pComponentName] = pFunction;
        }

        const Actor::Id GetNextActorId() { return m_nextActorId++; }
        size_t GetNumComponentCreator() { return m_actorComponentCreatorMap.size(); }

    private:
        std::shared_ptr<Actor> CreateActor(tinyxml2::XMLDocument& kDoc, const tinyxml2::XMLError& kError);
        std::unique_ptr<IActorComponent> CreateComponent(tinyxml2::XMLElement* pData, Actor* pOwner);
    };
}