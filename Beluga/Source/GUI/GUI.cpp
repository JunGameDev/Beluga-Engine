#include "GUI/GUI.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Graphics/Graphics.h"
#include <SDL.h>

using namespace Bel;

class SDLButton : public IUIButton
{
private:
    Rect m_destRect;
    Rect m_resourceRect;
    Color m_color;

public:
    SDLButton()
        : m_destRect({ 0, 0, 0, 0 })
        , m_resourceRect({ 0, 0, 0, 0 })
        , m_color({ 0, 0, 0, 0 })
    {
    }

    virtual ~SDLButton() override
    {
    }

    virtual bool Initialize()
    {
        return true;
    }

    virtual bool CheckMousePosition(int32_t x, int32_t y) override
    {
        SDL_Point p{ x, y };
        SDL_Rect dstRect{ m_destRect.m_x, m_destRect.m_y, m_destRect.m_w, m_destRect.m_h };
        bool selectedNow = SDL_PointInRect(&p, &dstRect);

        if (selectedNow)
        {
            m_handleClickEnter();
        }

        return selectedNow;
    }

    virtual Rect GetSourceRect() override
    {
        return m_resourceRect;
    }

    virtual Rect GetDestRect() override
    {
        return m_destRect;
    }

    virtual Color GetColor() override
    {
        return m_color;
    }

    virtual void SetDestRect(Rect rect) override
    {
        m_destRect = rect;
    }

    virtual void SetResourceRect(Rect rect) override
    {
        m_resourceRect = rect;
    }

    virtual void SetColor(Color color) override
    {
        m_color = color;
    }
};

std::unique_ptr<IUIButton> IUIButton::Create()
{
    return std::make_unique<SDLButton>();
}