#pragma once

#include <memory>
#include <cstddef>

namespace Bel
{
    class IWindow;
    class ITexture2D;
    class Resource;

    enum class RenderMode
    {
        kRenderFull,
        kRenderFree,
        kRenderCount
    };

    enum class ShapeType
    {
        kRectangle,
        kSquare,
        kLine,
        kShapeCount
    };  

    struct Color
    {
        uint8_t m_r;
        uint8_t m_g;
        uint8_t m_b;
        uint8_t m_a;
    }; // 4 bytes

    struct Rect
    {
        int32_t m_x;
        int32_t m_y;
        int32_t m_w;
        int32_t m_h;
    }; // 16 bytes
    
    struct Point
    {
        int m_x;
        int m_y;
    }; // 8 bytes

    template<typename Type>
    struct TPoint
    {
        Type m_x;
        Type m_y;
    };

    struct Line
    {
        Point m_src;
        Point m_dest;
    }; // 16 bytes

    struct Particle
    {
        ITexture2D* m_pTexture;
        
        float m_x;
        float m_y;
        float m_w;
        float m_h;

        float m_rotation;
        
        float m_xVel;
        float m_yVel;
        float m_rotVel;

        bool m_useGravity;
        bool m_inUse;
        bool m_fadeAway;
        
        float m_lifetime;
        float m_startingLifetime;

        Color m_color;

        Particle* m_pNext;
        Particle* m_pPrev;
    };

    enum class RenderFlip
    {
        FLIP_NONE       = 0x00000000,       /* Do not flip       */
        FLIP_HORIZONTAL = 0x00000001,       /* flip horizontally */
        FLIP_VERTICAL   = 0x00000002        /* flip vertically   */
    };

    class ITexture2D
    {
    protected:
        Rect m_srcRect;
        Rect m_destRect;
        RenderFlip m_flip;
        Point m_center;

    public:
        virtual ~ITexture2D() {}

        virtual void* GetNativeTexture() const  = 0;
        virtual Rect GetSourceRect() const      = 0;
        virtual Rect GetDestRect() const        = 0;
        virtual const Point GetCenter() const   = 0;
        virtual float GetAngle()                = 0;

        virtual void SetResourceRect(int x, int y, int w, int h)    = 0;
        virtual void SetDestRect(int x, int y, int w, int h)        = 0;
        virtual void SetCenter(int x, int y)                        = 0;

        virtual void SetImageSize(int w, int h)             = 0;
        virtual void SetRenderStartPosition(int x, int y)   = 0;

        virtual void SetTextureColor(uint8_t r, uint8_t g, uint8_t b)   = 0;
        virtual void SetTextureAlpha(uint8_t alpha)                     = 0;
        virtual void SetAngle(float angle)                              = 0;
        virtual void AddAngle(float angle)                              = 0;

        void SetRenderFlip(const RenderFlip& flip) { m_flip = flip; }
        RenderFlip& GetRenderFlip()         { return m_flip; }
    };

    class IGraphics
    {
    public:
        virtual ~IGraphics() {}

        virtual bool Initialize(IWindow* pWindow)             = 0;
        virtual bool StartDrawing()                           = 0;
        virtual bool RenderFillRect(Rect& rect, Color& color) = 0;
        virtual bool RenderRect(Rect& rect, Color& color)     = 0;
        virtual bool RenderCircle(int centerX, int centerY, float radius) = 0;
        virtual bool RenderLine(Point& src, Point& dest, const Color& kColor) = 0;

        virtual void Clear()                            = 0;
        virtual void ClearWithColor(const Color& color) = 0;
        virtual void EndDrawing()                       = 0;

        virtual void SetBackground(const Color& color) = 0;

        virtual std::shared_ptr<ITexture2D> LoadTextureFromCache(const char* pFileName) = 0;
        virtual std::shared_ptr<ITexture2D> LoadTextureDirectly(const char* pFileName) = 0;
        virtual bool DrawTexture(ITexture2D* pTexture, RenderMode mode = RenderMode::kRenderFree) = 0;
        virtual bool DrawTexture(ITexture2D* pTexture, Rect src, Rect dest) = 0;

        static std::unique_ptr<IGraphics> Create();
    };

    static inline bool PointInRect(const Point& pPoint, const Rect& pRect)
    {
        return ((pPoint.m_x >= pRect.m_x) && (pPoint.m_x < (pRect.m_x + pRect.m_w)) &&
            (pPoint.m_y >= pRect.m_y) && (pPoint.m_y < (pRect.m_y + pRect.m_h))) ? true : false;
    }
}
