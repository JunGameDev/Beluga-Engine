#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Graphics/Graphics.h"
#include "Resources/Resource.h"
#include "Parshing/tinyxml2.h"
#include "Actors/Actor.h"

static const Bel::IActorComponent::Id kTileId = Bel::IActorComponent::HashName("TileSetComponent");

namespace Bel
{
    class TileSet
    {
    private:
        uint32_t m_tileWidth;
        uint32_t m_tileHeight;
        uint32_t m_columns;
        uint32_t m_firstGridId;
        uint32_t m_margin;

        float m_ratio;
        std::string m_name;

        std::shared_ptr<ITexture2D> m_pTexture;

    public:
        TileSet()
            : m_tileWidth(0)
            , m_tileHeight(0)
            , m_columns(0)
            , m_firstGridId(0)
            , m_margin(0)
            , m_ratio(1.f)
        {
        }

        bool Initialize(tinyxml2::XMLElement* pElement, ResourceCache* pResCache, std::string mapFilepath);

        std::shared_ptr<ITexture2D> GetTexture() const { return m_pTexture; }
        uint32_t GetTileWidth()      const  { return m_tileWidth; }
        uint32_t GetTileHeight()     const  { return m_tileHeight; }
        uint32_t GetFirstGridId()    const  { return m_firstGridId; }
        float    GetRenderingRatio() const  { return m_ratio; }

        const std::string& GetName() { return m_name; }
        void GetSrc(uint32_t tileIndex, uint32_t& srcX, uint32_t& srcY);
    };

    class Layer
    {
    private:
        std::vector<uint32_t> m_tiles;
        uint32_t m_width;
        std::string m_usedTileSet;

    public:
        Layer()
            : m_width(0)
        {
        }

        bool Initialize(tinyxml2::XMLElement* pElement);

        uint32_t GetWidth() const { return m_width; }
        std::vector<uint32_t>& GetTiles() { return m_tiles; }
        const std::string GetUsedTileSet() { return m_usedTileSet; }
    };

    class TileSetComponent : public IActorComponent
    {
        using TextureMap = std::unordered_map<std::string, std::shared_ptr<ITexture2D>>;

    public:
        // Track the source and destination points of our tiles
        struct Tile
        {
            uint32_t m_srcX;
            uint32_t m_srcY;
            int32_t m_destX;
            int32_t m_destY;
            std::string m_pName;
            bool m_animated;
        };

    private:
        TextureMap m_textures;
        std::vector<Tile> m_tiles;
        uint32_t m_width;
        uint32_t m_height;
        float m_ratio; // Ratio for rendering.

    public:
        TileSetComponent(Actor* pOwner, const char* pName)
            : IActorComponent(pOwner, pName)
            , m_width(0)
            , m_height(0)
            , m_ratio(1)
        {
        }
        virtual ~TileSetComponent() {}

        virtual bool Initialize(tinyxml2::XMLElement* pData) override { return true; }
        virtual void Render(IGraphics* pGraphics);
        bool Initialize(uint32_t numTiles, uint32_t width, uint32_t height, float ratio);
        void AddTile(const Tile& tile) { m_tiles.push_back(tile); }
        void AddTileSet(const char* pName, std::shared_ptr<ITexture2D> pTexture);
        void SetRenderingRatio(float value) { m_ratio = value; }
    };

    //-----------------------------------------------------------------------------------------
    // Level
    // 
    // [ Description ] 
    //     - A container for holding htlayer and tilset info.
    //
    // TODO:
    //   [] Add logging. 
    //-----------------------------------------------------------------------------------------
    class Level
    {
    private:
        // With multiple tilesets 
        std::unordered_map<std::string, TileSet> m_tileData;
        std::vector<Layer> m_layers;

    public:
        std::shared_ptr<Actor> LoadTiles(ResourceCache* pResCache, std::string filePath);
        std::vector<std::shared_ptr<Actor>> LoadObjects(ResourceCache* pResCache, std::string fildPath);

    private:
        void ParseObjects(ResourceCache* pResCache, tinyxml2::XMLElement* pElement, std::vector<std::shared_ptr<Actor>>& actors);
        void ParseObject(ResourceCache* pResCache, tinyxml2::XMLElement* pObject, tinyxml2::XMLElement* pElement, std::vector<std::shared_ptr<Actor>>& actors);
        bool HasTileSet(const char* pName) const;
    };
}