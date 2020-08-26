#include "Actors/Actor.h"
#include "Resources/Resource.h"
#include "Core/Layers/ApplicationLayer.h"

using namespace Bel;
using namespace tinyxml2;

IActorComponent* Actor::GetComponent(IActorComponent::Id id)
{
    if (this == nullptr)
        return nullptr;

    // To Do: Consider to switch from using vector to hash map 
    auto itr = m_components.find(id);
    if (itr != m_components.end())
    {
        return itr->second.get();
    }
    return nullptr;
}

bool Actor::Initialize(XMLElement* pData)
{
    return true;
}

bool Actor::PostInit()
{
    for (auto& component : m_components)
    {
        if (!component.second->PostInit())
            return false;
    }
    return true;
}

void Actor::Destroy()
{
    auto gameLayer = ApplicationLayer::GetInstance()->GetGameLayer();
    gameLayer->DestroyActor(m_id);
}

void Actor::Update(float delta)
{
    if (this == nullptr)
        return;

    for (auto& component : m_components)
    {
        component.second->Update(delta);
    }
}

void Actor::Render(IGraphics* pGraphics)
{
    if (this == nullptr)
        return;

    for (auto& component : m_components)
    {
        component.second->Render(pGraphics);
    }
}

void Actor::AddComponent(std::unique_ptr<IActorComponent> pComponent)
{
    if (pComponent != nullptr)
    {
        m_components[pComponent->GetId()] = std::move(pComponent);
    }
}

bool Actor::HasComponent(IActorComponent::Id id)
{
    auto itr = m_components.find(id);
    if (itr != m_components.end())
    {
        return true;
    }

    return false;
}

bool Actor::IsName(const char* pName)
{
    if (m_name.empty() || (m_name != pName))
        return false;

    return true;
}

void Actor::RegisterWithScript()
{
    for (auto& pair : m_components)
    {
        pair.second->RegisterWithScript();
    }
}

std::shared_ptr<Actor> ActorFactory::CreateActor(tinyxml2::XMLDocument& doc, const tinyxml2::XMLError& kError)
{
    Logging& log = ApplicationLayer::GetInstance()->GetLogging();

    std::shared_ptr<Actor> pActor(new Actor(GetNextActorId()));

    tinyxml2::XMLElement* pRoot = doc.FirstChildElement();
    if (!pActor->Initialize(pRoot))
    {
        log.Log(Logging::SeverityLevel::kLevelWarn, "Unable to initialize actor: ", false);
        log.Log(Logging::SeverityLevel::kLevelWarn, pRoot->Name());
        return nullptr;
    }

    for (XMLElement* pElement = pRoot->FirstChildElement(); pElement;
        pElement = pElement->NextSiblingElement())
    {
        pActor->AddComponent(CreateComponent(pElement, pActor.get()));
    }

    if (!pActor->PostInit())
    {
        log.Log(Logging::SeverityLevel::kLevelWarn, "Unable to post init actor: ", false);
        log.Log(Logging::SeverityLevel::kLevelWarn, pRoot->Name());
        return nullptr;
    }

    return pActor;
}


std::shared_ptr<Actor> ActorFactory::CreateActorWithEmpty()
{
    return  std::make_shared<Actor>(GetNextActorId());
}

std::shared_ptr<Actor> ActorFactory::CreateActorByFileName(const std::string_view& fileName)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(fileName.data());
    Logging& log = ApplicationLayer::GetInstance()->GetLogging();

    if (error != XML_SUCCESS)
    {
        log.Log(Logging::SeverityLevel::kLevelWarn, "Unable to load file: ", false);
        log.Log(Logging::SeverityLevel::kLevelWarn, fileName.data());
        log.Log(Logging::SeverityLevel::kLevelWarn, tinyxml2::XMLDocument::ErrorIDToName(error));
        return nullptr;
    }

    return CreateActor(doc, error);
}

std::shared_ptr<Actor> Bel::ActorFactory::CreateActorByResource(std::shared_ptr<ResourceHandle> pResource)
{
    tinyxml2::XMLDocument doc;
    XMLError error = doc.Parse(pResource->GetData().data(), pResource->GetData().size());
    Logging& log = ApplicationLayer::GetInstance()->GetLogging();

    if (error != XML_SUCCESS)
    {
        log.Log(Logging::SeverityLevel::kLevelWarn, "Unable to load file: ", false);
        log.Log(Logging::SeverityLevel::kLevelWarn, pResource->GetName().c_str());
        log.Log(Logging::SeverityLevel::kLevelWarn, tinyxml2::XMLDocument::ErrorIDToName(error));

        return nullptr;
    }

    return CreateActor(doc, error);
}

std::unique_ptr<IActorComponent> ActorFactory::CreateComponent(XMLElement* pData, Actor* pOwner)
{
    const char* pName = pData->Name();
    std::unique_ptr<IActorComponent> pComponent;
    auto createItr = m_actorComponentCreatorMap.find(pName);
    if (createItr != m_actorComponentCreatorMap.end())
    {
        pComponent = std::move(createItr->second(pOwner, pName));
    }
    else
    {
        LOG_FATAL("Unable to find creator for component : ", false);
        LOG_FATAL(pName);

        return nullptr;
    }

    if (pComponent != nullptr && !pComponent->Initialize(pData))
    {
        return nullptr;
    }

    return pComponent;
}