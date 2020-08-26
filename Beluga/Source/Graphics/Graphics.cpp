#include "Core/Layers/ApplicationLayer.h"
#include <SDL_image.h>
#include <SDL.h>

#include "Graphics/Graphics.h"

using namespace Bel;

class SDLTexture2D : public ITexture2D
{
private:
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_pTexture;
    float m_angle;

public:
    SDLTexture2D()
        : m_pTexture(nullptr, nullptr)
        , m_angle(0)
    {
    }
    virtual ~SDLTexture2D() {}

    virtual void* GetNativeTexture() const override { return m_pTexture.get(); }
    virtual Rect GetSourceRect() const override { return m_srcRect; }
    virtual Rect GetDestRect() const override { return m_destRect; }
    virtual const Point GetCenter() const override { return m_center; }
    virtual float GetAngle() override { return m_angle; }

    bool Initialize(SDL_Renderer* pRenderer, SDL_Surface* pSurface)
    {
        m_pTexture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
            SDL_CreateTextureFromSurface(pRenderer, pSurface), &SDL_DestroyTexture);

        if (m_pTexture == nullptr)
        {
            LOG_ERROR("Unable to create texture");
            LOG_ERROR(IMG_GetError());
            return false;
        }

        m_flip = RenderFlip::FLIP_NONE;
        m_center = { 0, 0 };
        return true;
    }

    virtual void SetResourceRect(int x, int y, int w, int h) override
    {
        m_srcRect.m_x = x;
        m_srcRect.m_y = y;
        m_srcRect.m_w = w;
        m_srcRect.m_h = h;
    }
    virtual void SetDestRect(int x, int y, int w, int h) override
    {
        m_destRect.m_x = x;
        m_destRect.m_y = y;
        m_destRect.m_w = w;
        m_destRect.m_h = h;
    }
    virtual void SetCenter(int x, int y) override
    {
        m_center = { x, y };
    }
    virtual void SetImageSize(int w, int h) override
    {
        m_destRect.m_w = w;
        m_destRect.m_h = h;
    }
    virtual void SetRenderStartPosition(int x, int y) override
    {
        m_destRect.m_x = x;
        m_destRect.m_y = y;
    }
    virtual void SetTextureColor(uint8_t r, uint8_t g, uint8_t b) override
    {
        SDL_SetTextureColorMod(m_pTexture.get(), r, g, b);
    }
    virtual void SetTextureAlpha(uint8_t alpha)
    {
        SDL_SetTextureAlphaMod(m_pTexture.get(), alpha);
    }
    virtual void SetAngle(float angle)
    {
        m_angle = angle;
    }
    virtual void AddAngle(float angle)
    {
        m_angle += angle;
    }
};

class SDLGraphics : public IGraphics
{
private:
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_pRenderer;
    Color m_background;

public:
    SDLGraphics()
        : m_pRenderer(nullptr, nullptr)
        , m_background({255, 255, 255, 255})
    {
    }

    virtual bool Initialize(IWindow* pWindow) override
    {
        SDL_Window* pSDLWindow = reinterpret_cast<SDL_Window*>(pWindow->GetNativeWindow());
        
        // Render Mode
        // SDL_RENDERER_ACCELERATED :
        // SDL_RENDERER_PRESENTVSYNC :
        m_pRenderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(SDL_CreateRenderer(pSDLWindow, -1, SDL_RENDERER_PRESENTVSYNC), &SDL_DestroyRenderer);

        if (m_pRenderer == nullptr)
        {
            LOG_ERROR("Unable to initialize SDL Renderer");
            LOG_ERROR(SDL_GetError());
            return false;
        }

        return true;
    }
    
    virtual bool StartDrawing() override
    {
        SDL_SetRenderDrawColor(m_pRenderer.get(), m_background.m_r, m_background.m_g, m_background.m_b, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(m_pRenderer.get());
        return true;
    }

    virtual bool RenderFillRect(Rect& rect, Color& color) override
    {
        if (m_pRenderer == nullptr)
            return false;
        SDL_Rect sdlRect = { rect.m_x, rect.m_y, rect.m_w, rect.m_h };
        SDL_SetRenderDrawColor(m_pRenderer.get(), color.m_r, color.m_g, color.m_b, color.m_a);
        return !SDL_RenderFillRect(m_pRenderer.get(), &sdlRect);
    }

    virtual bool RenderLine(Point& src, Point& dest, const Color& kColor) override
    {
        if (m_pRenderer == nullptr)
            return false;
        SDL_SetRenderDrawColor(m_pRenderer.get(), kColor.m_r, kColor.m_g, kColor.m_b, kColor.m_a);
        return !SDL_RenderDrawLine(m_pRenderer.get(), src.m_x, src.m_y, dest.m_x, dest.m_y);
    }

    virtual bool RenderRect(Rect& rect, Color& color) override
    {
        if (m_pRenderer == nullptr)
            return false;

        SDL_SetRenderDrawColor(m_pRenderer.get(), color.m_r, color.m_g, color.m_b, color.m_a);
        SDL_Rect sdlRect = { rect.m_x, rect.m_y, rect.m_w, rect.m_h };
        return !SDL_RenderDrawRect(m_pRenderer.get(), &sdlRect);
    }

    virtual bool RenderCircle(int centerX, int centerY, float radius) override
    {
        double error = (double)-radius;
        double x = (double)radius;
        double y = (double)0.5;
        Color lineColor{ 255, 0, 0, 255 };

        while (x >= y)
        {
            SetPixel((int)(centerX + x), (int)(centerY + y), lineColor);
            SetPixel((int)(centerX + y), (int)(centerY + x), lineColor);

            if (x != 0)
            {
                SetPixel((int)(centerX - x), (int)(centerY + y), lineColor);
                SetPixel((int)(centerX + y), (int)(centerY - x), lineColor);
            }

            if (y != 0)
            {
                SetPixel((int)(centerX + x), (int)(centerY - y), lineColor);
                SetPixel((int)(centerX - y), (int)(centerY + x), lineColor);
            }

            if (x != 0 && y != 0)
            {
                SetPixel((int)(centerX - x), (int)(centerY - y), lineColor);
                SetPixel((int)(centerX - y), (int)(centerY - x), lineColor);
            }

            error += y;
            ++y;
            error += y;

            if (error >= 0)
            {
                --x;
                error -= x;
                error -= x;
            }
        }
        return true;
    }

    bool SetPixel(int x, int y, Color color)
    {
        SDL_SetRenderDrawColor(m_pRenderer.get(), color.m_r, color.m_g, color.m_b, color.m_a);
        return !SDL_RenderDrawPoint(m_pRenderer.get(), x, y);
    }

    virtual void EndDrawing() override
    {
        SDL_RenderPresent(m_pRenderer.get());
    }

    virtual void SetBackground(const Color& color) override
    {
        m_background = color;
    }

    virtual void Clear() override
    {
        ClearWithColor({ 0, 0, 0, 255 });
    }

    virtual void ClearWithColor(const Color& color) override
    {
        SetBackground(color);
        StartDrawing();
    }

    virtual std::shared_ptr<ITexture2D> LoadTextureFromCache(const char* pFileName) override
    {
        if (m_pRenderer == nullptr)
        {
            return nullptr;
        }

        auto pResCache = ApplicationLayer::GetInstance()->GetGameLayer()->GetResourceCache();
        auto pResource = pResCache->GetHandle(&Resource(pFileName));

        return LoadTexture(IMG_Load_RW(SDL_RWFromMem(pResource->GetData().data(), static_cast<int>(pResource->GetData().size())), 0), pFileName);
    }
    
    virtual std::shared_ptr<ITexture2D> LoadTextureDirectly(const char* pFileName) override
    {
        if (m_pRenderer == nullptr)
        {
            return nullptr;
        }

        return LoadTexture(IMG_Load(pFileName), pFileName);
    }

    virtual bool DrawTexture(ITexture2D* pTexture, RenderMode mode) override
    {
        SDL_Texture* pSDLTexture = reinterpret_cast<SDL_Texture*>(pTexture->GetNativeTexture());

        switch(mode)
        {
        case RenderMode::kRenderFull:
        {
            if (SDL_RenderCopy(m_pRenderer.get(), pSDLTexture, nullptr, nullptr))
            {
                LOG_ERROR("Unable to draw texture");
                LOG_ERROR(IMG_GetError());

                return false;
            }
        }
        break;
        case RenderMode::kRenderFree:
        {
            Rect srcRect = pTexture->GetSourceRect();
            Rect destRect = pTexture->GetDestRect();
            Point center = pTexture->GetCenter();
            const Vector2<int>& camera = Camera.GetRenderingPoint();
            
            SDL_Rect sdlDestRect = 
            { 
                destRect.m_x - center.m_x - camera.m_x, 
                destRect.m_y - center.m_y - camera.m_y, 
                destRect.m_w, 
                destRect.m_h 
            };
            SDL_Rect sdlSrcRect = { srcRect.m_x, srcRect.m_y, srcRect.m_w, srcRect.m_h };
            SDL_Point sdlCenter = { pTexture->GetCenter().m_x, pTexture->GetCenter().m_y };
            float angle = pTexture->GetAngle();

            SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(pTexture->GetRenderFlip());
            if (SDL_RenderCopyEx(m_pRenderer.get(), pSDLTexture, &sdlSrcRect, &sdlDestRect, angle, nullptr, flip))
            {
                LOG_ERROR("Unable to draw texture");
                LOG_ERROR(IMG_GetError());

                return false;
            }
#if defined(DEBUG)
            Rect test = { sdlDestRect.x, sdlDestRect.y, destRect.m_w, destRect.m_h };
            Color test2 = { 0, 255, 0, 0 };
            RenderRect(test, test2);
#endif
            //Rect testCenter = { center.m_x - camera.m_x, center.m_y - camera.m_y, 4, 4 };
            //Color test3 = { 255, 255, 0, 0 };
            //RenderRect(testCenter, test3);
        }
        break;
        }
        return true;
    }

    virtual bool DrawTexture(ITexture2D* pTexture, Rect src, Rect dest) override
    {
        SDL_Texture* pSDLTexture = reinterpret_cast<SDL_Texture*>(pTexture->GetNativeTexture());
        const Vector2<int>& camera = Camera.GetRenderingPoint();

        SDL_Rect sdlSrcRect = 
        { 
            static_cast<int>(src.m_x), 
            static_cast<int>(src.m_y),
            static_cast<int>(src.m_w),
            static_cast<int>(src.m_h)
        };
        
        SDL_Rect sdlDestRect = 
        { 
            static_cast<int>(dest.m_x - camera.m_x),
            static_cast<int>(dest.m_y - camera.m_y),
            static_cast<int>(dest.m_w),
            static_cast<int>(dest.m_h)
        };

        SDL_Point sdlCenter = 
        { 
            pTexture->GetCenter().m_x, 
            pTexture->GetCenter().m_y 
        };

        SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(pTexture->GetRenderFlip());
        
        if (SDL_RenderCopyEx
               (
                    m_pRenderer.get(), 
                    pSDLTexture, 
                    &sdlSrcRect, 
                    &sdlDestRect, 
                    0, 
                    &sdlCenter, 
                    flip
                )
           )
        {
            LOG_ERROR("Unable to draw texture");
            LOG_ERROR(IMG_GetError());

            return false;
        }
        return true;
    }

private:
    std::shared_ptr<ITexture2D> LoadTexture(SDL_Surface* pSdlSurface, const char* pFileName)
    {
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> pSurface(pSdlSurface, &SDL_FreeSurface); 
        if (pSurface == nullptr)
        {
            LOG_ERROR("Unable to load image", false);
            LOG_ERROR(pFileName);
            LOG_ERROR(IMG_GetError());
            return nullptr;
        }

        std::shared_ptr<ITexture2D> pTexture = std::make_shared<SDLTexture2D>();
        if (!static_cast<SDLTexture2D*>(pTexture.get())->Initialize(m_pRenderer.get(), pSurface.get()))
        {
            return nullptr;
        }

        return pTexture;
    }
};

std::unique_ptr<IGraphics> IGraphics::Create()
{
    return std::make_unique<SDLGraphics>();
}
