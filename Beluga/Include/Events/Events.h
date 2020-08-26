#pragma once
#include <ostream>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string_view>
#include "Core/Util/GUID_Helper.h"

namespace Bel
{
    typedef GUID EventType;

    class IEvent
    {
    public:
        virtual ~IEvent() {}
        virtual EventType GetEventType() const = 0;
        virtual const std::string_view GetName() const = 0;
    };

    using EventListenerDelegate = std::function<void(IEvent*)>;

    class EventManager
    {
        constexpr static std::size_t kNumQueues = 2;
    public:
        using EventListenerList = std::vector<EventListenerDelegate>;
        using EventListenerMap  = std::unordered_map<EventType, EventListenerList>;
        using EventQueue        = std::vector<std::unique_ptr<IEvent>>;

    private:
        int                 m_activeQueue;
        EventListenerMap    m_eventListeners;
        EventQueue          m_queues[kNumQueues];

    public:
        EventManager();
        std::size_t AddEventListener(const EventType& type, EventListenerDelegate listener);
        void RemoveEventListener(EventType type, std::size_t index);

        void QueueEvent(std::unique_ptr<IEvent> pEvent);
        void AbortEvent(EventType type, bool allOfType = false);

        void ProcessEvents();
        void TriggerEvent(std::unique_ptr<IEvent> pEvent);
    };
}
