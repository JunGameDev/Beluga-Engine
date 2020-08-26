#pragma once
#include <array>

namespace Bel
{
    class IGameLayer;

    class IKeyboard
    {
    public:
        enum KeyCode : int
        {
            kA,
            kB,
            kC,
            kD,
            kE,
            kF,
            kG,
            kH,
            kI,
            kJ,
            kK,
            kL,
            kM,
            kN,
            kO,
            kP,
            kQ,
            kR,
            kS,
            kT,
            kU,
            kV,
            kW,
            kX,
            kY,
            kZ,
            kKeypad1,
            kKeypad2,
            kKeypad3,
            kKeypad4,
            kKeypad5,
            kKeypad6,
            kKeypad7,
            kKeypad8,
            kKeypad9,
            kKeypad0,
            kF1,
            kF2,
            kF3,
            kF4,
            kF5,
            kF6,
            kF7,
            kF8,
            kF9,
            kF10,
            kF11,
            kF12,
            kCTRL_L,
            kCTRL_R,
            kAlt,
            kCapsLock,
            kHome,
            kSpace,
            kEnd,
            kSrcLk,
            kIns,
            kDel,
            kLeftBracket,
            kRightBracket,
            kArrowLeft,
            kArrowRight,
            kArrowUp,
            kArrowDown,
            kEsc,
            kPgUp,
            kPgDown,
            kCount
        };
    protected:
        std::array<bool, KeyCode::kCount> m_keyState;
        std::array<bool, KeyCode::kCount> m_previouseKeyState;

    public:
        virtual ~IKeyboard() {}

        virtual bool Initialize()
        {
            m_keyState = { false };
            return true;
        }

        virtual void SetKeyState(KeyCode key, bool down)
        {
            if (key != KeyCode::kCount)
            {
                m_keyState[key] = down;
            }
        }

        virtual bool IsKeyDown(KeyCode key) { return m_keyState[key]; }
        virtual bool IsKeyPressed(KeyCode key)
        {
            return m_keyState[key] && !m_previouseKeyState[key];
        }

        virtual bool IsKeyReleased(KeyCode key)
        {
            return !m_keyState[key] && m_previouseKeyState[key];
        }

        virtual void NextFrame()
        {
            m_previouseKeyState = m_keyState;
        }

        static std::unique_ptr<IKeyboard> Create();
    };

    class IMouse
    {
    public:
        enum Button : int
        {
            kBtnLeft,
            kBtnRight,
            kBtnWheelClicked,
            kBtnWheelUp,
            kBtnWheelDown,
            kBtnWheelLeft,
            kBtnWheelRight,
            kBtnMax
        };

    protected:
        std::array<bool, kBtnMax> m_buttonState;
        std::array<bool, kBtnMax> m_previousButtonState;
        // Information about the state of mouse wheel.
        int32_t m_wheelX;
        int32_t m_wheelY;

    public:
        virtual ~IMouse() {}

        virtual bool Initialize() = 0;

        virtual void SetButtonState(uint32_t button, bool down) = 0;

        virtual bool IsButtonDown(Button button) = 0;
        virtual bool IsButtonPressed(Button button) = 0;
        virtual bool IsButtonReleased(Button button) = 0;
        virtual bool IsWheelUp() = 0;
        virtual bool IsWheelDown() = 0;

        virtual void SetWheelX(int32_t x) = 0;
        virtual void SetWheelY(int32_t y) = 0;

        virtual void SetPosToMouse(int32_t& x, int32_t& y) = 0;

        virtual void NextFrame() = 0;

        virtual void SetMousePosition(int32_t x, int32_t y) = 0;
        virtual int32_t GetMouseX() = 0;
        virtual int32_t GetMouseY() = 0;
        virtual int32_t GetWheelX() = 0;
        virtual int32_t GetWheelY() = 0;

        static std::unique_ptr<IMouse> Create();
    };

    class IGameController
    {
    public:
        enum PadButton : int
        {
            kBtnA,
            kBtnB,
            kBtnX,
            kBtnY,
            kTriggerL,
            kTriggerR,
            kBtnBumpperL,
            kBtnBumpperR,
            kBtnDpadUp,
            kBtnDpadDown,
            kBtnDpadRight,
            kBtnDpadLeft,
            kBtnStickLeft,
            kBtnStickRight,
            kBtnHome,
            kBtnMenu,
            kBtnView,
            kBtnMax
        };

    protected:
        int32_t m_axisX;
        int32_t m_axisY;

        float m_leftTrigger;
        float m_rightTrigger;

        std::array<bool, kBtnMax> m_buttonState;
        std::array<bool, kBtnMax> m_previousButtonState;

    public:
        virtual ~IGameController() {}

        virtual bool Initialize() = 0;
        virtual void NextFrame() = 0;

        virtual void SetButtonState(uint32_t button, bool down) = 0;

        virtual bool IsButtonDown(PadButton button) = 0;
        virtual bool IsButtonPressed(PadButton button) = 0;
        virtual bool IsButtonReleased(PadButton button) = 0;

        virtual int32_t GetAxisX() = 0;
        virtual int32_t GetAxisY() = 0;
        virtual std::size_t GetAxisTotal() = 0;
        virtual std::size_t GetButtonTotal() = 0;
        virtual const char* GetControllerName() = 0;

        bool GetButtonState(size_t idx) const { return m_buttonState[idx]; }
        static std::unique_ptr<IGameController> Create();
    };
}
