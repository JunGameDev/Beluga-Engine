#include <assert.h>
#include "Events/Events.h"
#include "Core/Layers/ApplicationLayer.h"

using namespace Bel;

EventManager::EventManager()
    : m_activeQueue(0)
{

}

std::size_t EventManager::AddEventListener(const EventType& type, EventListenerDelegate listener)
{
    EventListenerList& listeners = m_eventListeners[type];
    
    auto emplaceIt = std::find(listeners.begin(), listeners.end(), nullptr);
    if (emplaceIt == listeners.end())
    {
        emplaceIt = listeners.emplace(emplaceIt, listener);
    }
    else
    {
        (*emplaceIt) = listener;
    }
    
    return emplaceIt - listeners.begin();
}

void EventManager::RemoveEventListener(EventType type, std::size_t index)
{
    auto& listeners = m_eventListeners[type];
    if (index < listeners.size())
    {
        listeners[index] = nullptr;
    }
}

void EventManager::QueueEvent(std::unique_ptr<IEvent> pEvent)
{
    assert(m_activeQueue >= 0);
    assert(m_activeQueue < kNumQueues);

    auto listener = m_eventListeners.find(pEvent->GetEventType());

    if (listener != m_eventListeners.end())
    {
        m_queues[m_activeQueue].emplace_back(std::move(pEvent));
    }
}

void EventManager::ProcessEvents()
{
    // Move this so that any new events will be processed next frame
    int queueToProcess = m_activeQueue;
    m_activeQueue = (m_activeQueue + 1) % kNumQueues;
    m_queues[m_activeQueue].clear();

    // Move this so that new events will be processed next frame
    auto processingQueue = std::move(m_queues[queueToProcess]);

    for (auto& pEvent : processingQueue)
    {
        auto& listeners = m_eventListeners[pEvent->GetEventType()];
        for (auto& listener : listeners)
        {
            if (listener != nullptr)
            {
                listener(pEvent.get());
            }
        }
    }
}

void EventManager::AbortEvent(EventType type, bool allOfType)
{
    assert(m_activeQueue >= 0);
    assert(m_activeQueue < kNumQueues);

    EventListenerMap::iterator findIt = m_eventListeners.find(type);

    if (findIt != m_eventListeners.end())
    {
        EventQueue& eventQueue = m_queues[m_activeQueue];
        auto itr = eventQueue.begin();
        while (itr != eventQueue.end())
        {
            if ((*itr)->GetEventType() == type)
            {
                itr = eventQueue.erase(itr);
                if (!allOfType)
                    break;
            }
            else
            {
                ++itr;
            }
        }
    }
}

void EventManager::TriggerEvent(std::unique_ptr<IEvent> pEvent)
{
    auto& listeners = m_eventListeners[pEvent->GetEventType()];
    for (auto& listener : listeners)
    {
        if (listener != nullptr)
        {
            listener(pEvent.get());
        }
    }
}