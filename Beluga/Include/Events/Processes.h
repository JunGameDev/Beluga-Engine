#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace Bel
{
    class Actor;

    class IProcess
    {
    public:
        enum class State
        {
            UNINITIALIZED,
            REMOVED,
            RUNNING,
            PAUSED,
            SUCCEEDED,
            FAILED,
            ABORTED,
        };

    private:
        std::function<void()>       m_successCallback;
        std::function<void()>       m_failureCallback;
        std::function<void()>       m_abortCallback;
        std::shared_ptr<IProcess>   m_pChild;
        State m_state;

        uint32_t m_priority;

    public:
        IProcess()
            : m_state(State::UNINITIALIZED)
            , m_priority(0)
        {
        }
        virtual ~IProcess() {}

        virtual bool Initialize() { return true; }
        virtual void Update(float delta) = 0;

        void Succeeded()    { m_state = State::SUCCEEDED;   }
        void Failed()       { m_state = State::FAILED;      }
        void Aborted()      { m_state = State::ABORTED;     }
        void Pause()        { m_state = State::PAUSED;      }
        void Resume()       { m_state = State::RUNNING;     }
        
        bool IsRemoved() const { return (m_state == State::REMOVED); }
        bool IsPaused() const { return (m_state == State::PAUSED); }
        State GetState() const { return m_state; }

        std::shared_ptr<IProcess> PeekChild() { return m_pChild; }

        //===== Processes =====
        void AttachChild(std::shared_ptr<IProcess> pProcess);

        bool IsAlive() const;
        bool IsDead() const;

        std::shared_ptr<IProcess> RemoveChild();


        void SetSuccessCallback(std::function<void()> callback);
        void OnSuccess();
        void SetFailureCallback(std::function<void()> callback);
        void OnFailure();
        void SetAbortCallback(std::function<void()> callback);
        void OnAbort();
    };

    class ProcessManager
    {
    private:
        std::vector<std::shared_ptr<IProcess>> m_processes;

    public:
        ~ProcessManager();

        void UpdateProcesses(float delta);
        void AbortAllProcesses();
        void AttachProcess(std::shared_ptr<IProcess> pProcess);

        size_t GetProcessCount() const { return m_processes.size(); }

    private:
        void ClearAllProcesses();
    };
}
