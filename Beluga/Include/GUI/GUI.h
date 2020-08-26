#pragma once
#include <memory>
#include <functional>

namespace Bel
{
    struct Rect;
    struct Color;
    class ITexture2D;

    class IUIButton
    {
    protected:
        using OnClickEnter = std::function<void()>;
        using OnClickExit = std::function<void()>;

        OnClickEnter	m_handleClickEnter;
        OnClickExit     m_handleClickExit;

    public:
        virtual ~IUIButton() = 0 {};

        virtual bool Initialize() = 0;

        virtual Rect GetSourceRect() = 0;
        virtual Rect GetDestRect() = 0;
        virtual Color GetColor() = 0;

        virtual void SetDestRect(Rect rect) = 0;
        virtual void SetResourceRect(Rect rect) = 0;
        virtual void SetColor(Color color) = 0;

        virtual bool CheckMousePosition(int32_t x, int32_t y) = 0;

        void SetClickEnter(std::function<void()> func) { m_handleClickEnter = func; }
        void SetClickExit(std::function<void()> func) { m_handleClickExit = func; }

        static std::unique_ptr<IUIButton> Create();
    };
}

