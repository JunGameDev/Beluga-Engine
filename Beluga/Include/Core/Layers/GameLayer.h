#pragma once

#include <algorithm>

#include "Physics/Physics.h"
#include "View.h"
#include "Actors/Actor.h"
#include "Events/Processes.h"
#include "Events/Events.h"
#include "Core/Camera/Camera.h"
#include "Resources/Resource.h"
#include "Scripting/Scripting.h"
#include "Levels/Level.h"

namespace Bel
{
    class IGameLayer
    {
    protected:
        Camera2D m_camera;
        std::unordered_map<Actor::Id, std::shared_ptr<Actor>> m_actors;
        std::unordered_map<Actor::Id, std::shared_ptr<Actor>> m_guis;   // Better name???
        std::vector<std::unique_ptr<IView>> m_views;
        
        //std::vector<std::shared_ptr<Actor>> m_actorsToDelete;
        //std::vector<std::unique_ptr<IView>> m_viewsToDelete;
        
        ActorFactory     m_actorFactory;
        EventManager     m_eventManager;
        ProcessManager   m_processManager;
        ScriptingManager m_scriptingManager;

        std::unique_ptr<ResourceCache> m_pResCache;
        std::unique_ptr<IPhysicsManager> m_pPhysicsManager;
        std::shared_ptr<ContactListener> m_pContactListener;

        float m_xGravity;
        float m_yGravity;

    private:
        std::vector<std::unique_ptr<IView>> m_pendingViews;

    public:
        IGameLayer(float&& xGravity, float&& yGravity);
        virtual ~IGameLayer() 
        {
            m_views.clear();
            m_actors.clear();
        }
        virtual const char* GetGameName() const = 0;
        virtual void LoadLevel(IEvent* pEvent) = 0;

        virtual bool Initialize() 
        {
            m_scriptingManager.Initialize();
            m_pPhysicsManager = IPhysicsManager::Create(m_xGravity, -m_yGravity);
            m_pPhysicsManager->Initialize();
            AddPendingView();

            for (auto& pView : m_views)
            {
                if (!pView->Initialize())
                {
                    return false;
                }
            }

            return true;
        }

        virtual void AddView(std::unique_ptr<IView> pView)
        {
            m_pendingViews.emplace_back(std::move(pView));
        }

        virtual void Update(float delta)
        {
            AddPendingView();


            for (auto& pView : m_views)
            {
                pView->UpdateInput(delta);
            }

            m_eventManager.ProcessEvents();
            m_pPhysicsManager->Update(delta);
            m_processManager.UpdateProcesses(delta);

            for (auto& actor : m_actors)
            {
                if (actor.second == nullptr)
                    continue;

                actor.second->Update(delta);
            }

            for (auto& actor : m_guis)
            {
                if (actor.second == nullptr)
                    continue;

                actor.second->Update(delta);
            }

            for (auto& pView : m_views)
            {
                if (pView == nullptr)
                    continue;

                pView->ViewScene();
            }

#if defined(DEBUG)
            m_pPhysicsManager->DrawDebugLine();
#endif
        }

        virtual void RegisterWithLua()
        {
            m_scriptingManager.CreateTable(); // Table for logic

            m_scriptingManager.CreateTable(); // Table for actor
            m_scriptingManager.AddToTable("actors");
            
            m_scriptingManager.SetGlobal("g_logic");
        }
    
        void AddPendingView()
        {
            for (auto& pView : m_pendingViews)
            {
                m_views.emplace_back(std::move(pView));
            }
            m_pendingViews.clear();
        }

        void RemoveView(IView* pViewToRemove)
        {
            std::vector<std::unique_ptr<IView>>::iterator object 
                = std::find_if(m_views.begin(), m_views.end(), [&](std::unique_ptr<IView>& obj) { return obj.get() == pViewToRemove; });

            std::unique_ptr<IView> pView = std::move((*object));
            if (object != m_views.end())
            {
                m_views.erase(object);
                pView.reset();
            }
        }

        void FindViewToDelete(Actor::Id id)
        {
            std::shared_ptr<Actor> pActor = GetActor(id);
            assert(pActor != nullptr);
            
            auto pView = FindView(id);

            if (pView == nullptr)
                return;

            pView->Delete();
            RemoveView(pView);
        }

        IView* FindView(Actor::Id id)
        {
            std::shared_ptr<Actor> pActor = GetActor(id);

            if (pActor == nullptr)
                return nullptr;

            for (auto& pView : m_views)
            {
                if (pView == nullptr)
                    return nullptr;

                std::shared_ptr<Actor> pActorToCompare = pView->GetActor();
                if (pActor == pActorToCompare)
                {
                    return pView.get();
                }
            }

            return nullptr;
        }

        void AddActor(Actor::Id id, std::shared_ptr<Actor> pActor)
        {
            m_actors[id] = pActor;
        }

        void AddGUI(Actor::Id id, std::shared_ptr<Actor> pActor)
        {
            m_guis[id] = pActor;
        }

        void DestroyActor(Actor::Id id)
        {
            auto pActor = m_actors[id];

            if (pActor == nullptr)
                return;

            auto components = pActor->GetComponents();
            for (auto compIter = components->begin(); compIter != components->end(); ++compIter)
            {
                compIter->second.reset();
            }
            pActor->GetComponents()->clear();
            pActor.reset();
            m_actors.erase(id);
        }

        Actor::Id FindActorId(const char* pName)
        {
            for (auto& actor : m_actors)
            {
                if (actor.second->IsName(pName))
                    return actor.second->GetId();
            }

            return -1;
        }

        std::shared_ptr<Actor> FindActorWithName(const char* pName)
        {
            for (auto& pActor : m_actors)
            {
                if (pActor.second->IsName(pName))
                    return pActor.second;
            }

            return nullptr;
        }
            
        const std::unordered_map<Actor::Id, std::shared_ptr<Actor>>& GetActors()
        {
            return m_actors;
        }

        const std::unordered_map<Actor::Id, std::shared_ptr<Actor>>& GetGUIs()
        {
            return m_guis;
        }

        std::shared_ptr<Actor> GetActor(Actor::Id id)
        {
            auto pActorIter = m_actors.find(id);
            if (pActorIter != m_actors.end())
                return pActorIter->second;
         
            return nullptr;
        }

        EventManager&       GetEventManager()       { return m_eventManager; }
        ActorFactory&       GetActorFactory()       { return m_actorFactory; }
        Camera2D&           GetCamera()             { return m_camera; }
        ResourceCache*      GetResourceCache()      { return m_pResCache.get(); }
        ScriptingManager&   GetScriptingManager()   { return m_scriptingManager; }
        IPhysicsManager&    GetPhysicsManager()     { return *(m_pPhysicsManager.get()); }
    };
}

