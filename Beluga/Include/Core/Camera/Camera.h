#pragma once
#include "Graphics/Graphics.h"
#include "Core/Math/Vector2.h"

namespace Bel
{
    class Camera2D
    {
    private:
        uint32_t m_screenWidth;
        uint32_t m_screenHeight;

        //Point m_center;
        Vector2<int> m_center;
        Vector2<float> m_position;
    
    public:
        Camera2D()
            : m_screenWidth(0)
            , m_screenHeight(0)
            , m_center({0, 0})
            , m_position(0, 0)
        {

        }
        Camera2D(uint32_t screenWidth, uint32_t screenHeight)
            : m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight)
            , m_center({ static_cast<int>(screenWidth >> 1), static_cast<int>(screenHeight >> 1) })
            , m_position(0.f, 0.f)
        {
        }
        ~Camera2D() {};
        virtual bool Initialize()
        {
            return true;
        }

        virtual void MovePos(float x, float y)
        {
            m_position.m_x += x;
            m_position.m_y += y;
        }

        void SetPosition(float x, float y)
        {
            m_position.Set(x, y);
        }

        void RegisterWithLua();

        Vector2<float>& GetPosition() { return m_position; }
        Vector2<int>& GetCenter() { return m_center; }

        Vector2<int> GetRenderingPoint() const
        {
            return m_position - m_center;
        }
    };
}