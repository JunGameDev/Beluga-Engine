#pragma once
#include <memory>
#include <unordered_map>
#include "Core/Util/Guid_Helper.h"

namespace Bel
{
    struct Collision;
    class ICollider2D;
    class Actor;
    class IView
    {
    private:
        std::shared_ptr<Actor> m_pActor;

    protected:
        std::unordered_map<GUID, size_t> m_listenerIds;

    public:
        virtual ~IView() {}

        virtual bool Initialize() = 0;
        virtual void UpdateInput(float delta) = 0;
        virtual void ViewScene() = 0;
        virtual void Delete() = 0;

        void SetActor(std::shared_ptr<Actor> pActor) { m_pActor = pActor; }
        std::shared_ptr<Actor> GetActor() const { return m_pActor; }

        virtual void OnCollisionEnter(Collision& col) {}
        virtual void OnCollisionExit(Collision& col) {}

        virtual void OnTriggerEnter(Collision& col) {}
        virtual void OnTriggerExit(Collision& col) {}

        std::unordered_map<GUID, size_t> GetListenerIds() const { return m_listenerIds; }
    };
}
