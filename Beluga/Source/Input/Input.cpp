#include "Core/Layers/ApplicationLayer.h"
#include <SDL.h>

#include "Input/Input.h"
#include "Graphics/Graphics.h"

using namespace Bel;

class SDLMouse : public IMouse
{
private:
    Point m_cursor;

public:
    SDLMouse()
        : m_cursor({0, 0})
    {
       
    }
    virtual ~SDLMouse() override {}

    virtual bool Initialize() override
    {
        m_buttonState = { false };
        m_wheelX = 0;
        m_wheelY = 0;
        return true;
    }

    virtual void SetButtonState(uint32_t button, bool down) override
    {
        if (button != kBtnMax)
        {
            m_buttonState[button] = down;
        }
    }

    virtual bool IsWheelUp() override
    {
        if (m_wheelY > 0)
        {
            m_wheelY = 0;
            return true;
        }
        return false;
    }
    virtual bool IsWheelDown() override
    {
        if (m_wheelY < 0)
        {
            m_wheelY = 0;
            return true;
        }
        return false;
    }


    virtual bool IsButtonDown(Button button) override
    {
        return m_buttonState[button];
    }

    virtual void SetWheelX(int32_t x) override
    {
        m_wheelX = x;
    }
    virtual void SetWheelY(int32_t y) override
    {
        m_wheelY = y;
    }

    virtual bool IsButtonPressed(Button button) override
    {
        return m_buttonState[button] && !m_previousButtonState[button];
    }

    virtual bool IsButtonReleased(Button button) override
    {
        return !m_buttonState[button] && m_previousButtonState[button];
    }

    virtual void SetPosToMouse(int32_t& x, int32_t& y) override
    {
        x = m_cursor.m_x;
        y = m_cursor.m_y;
    }

    virtual void NextFrame() override
    {
        m_previousButtonState = m_buttonState;
    }

    virtual void SetMousePosition(int32_t x, int32_t y) override
    {
        m_cursor.m_x = x;
        m_cursor.m_y = y;
    }

    virtual int32_t GetMouseX() override
    {
        return m_cursor.m_x;
    }

    virtual int32_t GetMouseY() override
    {
        return m_cursor.m_y;
    }

    virtual int32_t GetWheelX() override
    {
        return m_wheelX;
    }

    virtual int32_t GetWheelY() override
    {
        return m_wheelY;
    }
};


//***********************************************************************
// Base interface controller class for supporting XBOX ONE controller.
// Each class using a different graphics library must be derived 
// from this class.
//***********************************************************************
class IXboxOneController : public IGameController
{
public:
    IXboxOneController() {}
    virtual ~IXboxOneController() override {};
    virtual bool Initialize() override = 0;
    virtual void NextFrame() override = 0;

    virtual void SetButtonState(uint32_t button, bool down) override = 0;

    virtual bool IsButtonDown(PadButton button) override = 0;
    virtual bool IsButtonPressed(PadButton button) override = 0;
    virtual bool IsButtonReleased(PadButton button) override = 0;

    virtual int32_t GetAxisX() override = 0;
    virtual int32_t GetAxisY() override = 0;
    virtual std::size_t GetAxisTotal() override = 0;
    virtual std::size_t GetButtonTotal() override = 0;

    virtual const char* GetControllerName() override { return "Xbox One Controller"; }
};

//***********************************************************
//                  SDLXboxOneController
//***********************************************************
class SDLXboxOneController final : public IXboxOneController
{
private:
    SDL_GameController* m_pController;

public:
    SDLXboxOneController()
        : m_pController(nullptr)
    {
        m_leftTrigger = { 0 };
        m_rightTrigger = { 0 };
    }
    virtual ~SDLXboxOneController() override
    {
       
    }
    virtual bool Initialize() override
    {
        if(SDL_Init(SDL_INIT_GAMECONTROLLER))
        {
            return false;
        }
        m_buttonState = { false };

        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            m_pController = SDL_GameControllerOpen(i);
        }

        if (m_pController == nullptr)
            return false;
        return true;
    }

    virtual void NextFrame() override
    {
        m_previousButtonState = m_buttonState;
    }

    virtual void SetButtonState(uint32_t button, bool down) override
    {
        if (button != kBtnMax)
        {
            m_buttonState[button] = down;
        }
    }

    virtual bool IsButtonDown(PadButton button) override
    {
        return m_buttonState[button];
    }

    virtual bool IsButtonPressed(PadButton button) override
    {
        return m_buttonState[button] && !m_previousButtonState[button];
    }
    
    virtual bool IsButtonReleased(PadButton button) override
    {
        return !m_buttonState[button] && m_previousButtonState[button];
    }
    
    virtual int32_t GetAxisX() override
    {
        return 0;
    }
    virtual int32_t GetAxisY() override
    {
        return 0;
    }
    virtual std::size_t GetAxisTotal() override
    {
        return 0;
    }
    virtual std::size_t GetButtonTotal() override
    {
        return 0;
    }
};


class TestController : public IGameController
{
public:
    virtual bool Initialize()   override { return true; }
    virtual void NextFrame()    override {}
    virtual void SetButtonState(uint32_t button, bool down) override {}
    
    virtual bool IsButtonDown(PadButton button)     override { return true; }
    virtual bool IsButtonPressed(PadButton button)  override { return true; }
    virtual bool IsButtonReleased(PadButton button) override { return true; }
    
    virtual int32_t GetAxisX() override { return 0; }
    virtual int32_t GetAxisY() override { return 0; }
    
    virtual std::size_t GetAxisTotal()   override { return 0; }
    virtual std::size_t GetButtonTotal() override { return 0; }
    
    virtual const char* GetControllerName() override { return "testcontrtoller"; }
};

std::unique_ptr<IKeyboard> IKeyboard::Create()
{
    return std::make_unique<IKeyboard>();
}

std::unique_ptr<IMouse> IMouse::Create()
{
    return std::make_unique<SDLMouse>();
}

std::unique_ptr<IGameController> IGameController::Create()
{
    return std::make_unique<SDLXboxOneController>();
}