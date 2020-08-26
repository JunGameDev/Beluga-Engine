#include "Events/Processes.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Actors/Actor.h"

using namespace Bel;

//**************************************************************************************************************************
//                                                      IProcess
//**************************************************************************************************************************
void IProcess::SetSuccessCallback(std::function<void()> callback)
{
    m_successCallback = callback;
}

void IProcess::OnSuccess()
{
    if (m_successCallback)
    {
        m_successCallback();
    }
}

void IProcess::SetFailureCallback(std::function<void()> callback)
{
    m_failureCallback = callback;
}

void IProcess::OnFailure()
{
    if (m_failureCallback)
    {
        m_failureCallback();
    }
}

void IProcess::SetAbortCallback(std::function<void()> callback)
{
    m_abortCallback = callback;
}

void IProcess::OnAbort()
{
    if (m_abortCallback)
    {
        m_abortCallback();
    }
}

void IProcess::AttachChild(std::shared_ptr<IProcess> pProcess)
{
    m_pChild = pProcess;
}

bool IProcess::IsAlive() const
{
    return m_state == State::RUNNING || m_state == State::PAUSED;
}

bool IProcess::IsDead() const
{
    return m_state == State::SUCCEEDED || m_state == State::FAILED || m_state == State::ABORTED;
}

std::shared_ptr<IProcess> IProcess::RemoveChild()
{
    auto pChild = m_pChild;
    m_pChild = nullptr;
    return pChild;
}

//**************************************************************************************************************************
//                                                    Process Manager
//**************************************************************************************************************************

ProcessManager::~ProcessManager()
{
    AbortAllProcesses();
    ClearAllProcesses();
}

void ProcessManager::UpdateProcesses(float delta)
{
    std::size_t processIndex = 0;
    while (processIndex != m_processes.size())
    {
        auto pProcess = m_processes[processIndex];

        if (pProcess->GetState() == IProcess::State::UNINITIALIZED)
        {
            if (pProcess->Initialize())
            {
                pProcess->Resume();
            }
            else
            {
                m_processes.erase(m_processes.begin() + processIndex);
                continue;
            }
        }

        if (pProcess->GetState() == IProcess::State::RUNNING)
        {
            pProcess->Update(delta);
        }

        if (pProcess->IsDead())
        {
            IProcess::State state = pProcess->GetState();
            if (state == IProcess::State::SUCCEEDED)
            {
                pProcess->OnSuccess();
                auto child = pProcess->RemoveChild();
                if (child)
                {
                    AttachProcess(child);
                }
            }
            else if (state == IProcess::State::FAILED)
            {
                pProcess->OnFailure();
            }
            else if (state == IProcess::State::ABORTED)
            {
                pProcess->OnAbort();
            }
            m_processes.erase(m_processes.begin() + processIndex);
            continue;
        }
        ++processIndex;
    }
}

void ProcessManager::AbortAllProcesses()
{
    for (auto pProcess : m_processes)
    {
        if (pProcess->IsAlive())
        {
            pProcess->Aborted();
            pProcess->OnAbort();
        }
    }
}

void ProcessManager::AttachProcess(std::shared_ptr<IProcess> pProcess)
{
    m_processes.emplace_back(pProcess);
}

void ProcessManager::ClearAllProcesses()
{
    m_processes.clear();
}
