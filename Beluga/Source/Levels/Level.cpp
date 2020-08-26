#include <algorithm>
#include <string_view>
#include "Levels/Level.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Physics/Physics.h"

using namespace Bel;
using namespace tinyxml2;

//-----------------------------------------------------------------------------------------
// TileSet
//-----------------------------------------------------------------------------------------
bool TileSet::Initialize(tinyxml2::XMLElement* pElement, ResourceCache* pResCache, std::string mapFilePath)
{
    m_firstGridId = pElement->IntAttribute("firstgid");
    const char* source = pElement->Attribute("source");

    // Get our current directory so relative paths work
    std::replace(mapFilePath.begin(), mapFilePath.end(), '\\', '/');
    std::size_t seperator = mapFilePath.find_last_of('/');
    if (seperator != std::string::npos)
    {
        mapFilePath = mapFilePath.substr(0, seperator + 1);
    }
    std::string filePath = mapFilePath + source;

    auto pTileSet = pResCache->GetHandle(&Resource(filePath));

    tinyxml2::XMLDocument doc;
    XMLError error = doc.Parse(pTileSet->GetData().data(), pTileSet->GetData().size());
    if (error != XML_SUCCESS)
    {
        return false;
    }

    XMLElement* pRoot = doc.FirstChildElement();
    if (strcmp(pRoot->Name(), "tileset") == 0)
    {
        m_columns = pRoot->UnsignedAttribute("columns");
        m_tileWidth = pRoot->UnsignedAttribute("tilewidth");
        m_tileHeight = pRoot->UnsignedAttribute("tileheight");
        m_margin = pRoot->UnsignedAttribute("margin");
        m_name = pRoot->Attribute("name");

        for (XMLElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
        {
            if (strcmp(pElement->Name(), "image") == 0)
            {
                std::string_view sourcePath = pElement->Attribute("source");
                filePath = sourcePath.substr(3);
                m_pTexture = ApplicationLayer::GetInstance()->GetGraphics()->LoadTextureFromCache(filePath.c_str());
            }
            else if (strcmp(pElement->Name(), "tile") == 0)
            {
                XMLElement* pProperties = pElement->FirstChildElement();
                for (XMLElement* pProperty = pProperties->FirstChildElement(); pProperty; pProperty = pProperty->NextSiblingElement())
                {
                    if (strcmp(pProperty->Attribute("name"), "RenderingRatio") == 0)
                    {
                        m_ratio = pProperty->FloatAttribute("value");
                    }
                }
            }
                
        }
    }
    else
    {
        return false;
    }
    return true;
}


void TileSet::GetSrc(uint32_t tileIndex, uint32_t& srcX, uint32_t& srcY)
{
    uint32_t index = tileIndex - 1; // 0 means no tile
    int32_t x = index % m_columns;
    int32_t y = index / m_columns;
    srcX = x * m_tileWidth;
    srcY = y * m_tileHeight;
}

//-----------------------------------------------------------------------------------------
// Layer
//-----------------------------------------------------------------------------------------
bool Layer::Initialize(tinyxml2::XMLElement* pElement)
{
    m_width = pElement->UnsignedAttribute("width");
    uint32_t height = pElement->UnsignedAttribute("height");

    auto childElem = pElement->FirstChildElement("properties");
    if (childElem != nullptr)
    {
        childElem = pElement->FirstChildElement("property");
        if (childElem != nullptr)
        {
            m_usedTileSet = childElem->Attribute("value");
        }
        else
        {
            m_usedTileSet = "";
        }
    }

    m_tiles.reserve(m_width * height);

    for (XMLElement* pLayerElement = pElement->FirstChildElement(); pLayerElement; pLayerElement = pLayerElement->NextSiblingElement())
    {
        if (strcmp(pLayerElement->Name(), "properties") == 0)
        {
            for (XMLElement* pInner = pLayerElement->FirstChildElement(); pInner; pInner = pInner->NextSiblingElement())
            {
                if (strcmp(pInner->Name(), "property") == 0)
                {
                    const char* pPropertyName = pInner->Attribute("name");
                    if(strcmp(pPropertyName, "UsedSpriteSheet") == 0)
                    {
                        m_usedTileSet = pInner->Attribute("value");
                    }
                }
            }
        }

        if (strcmp(pLayerElement->Name(), "data") == 0)
        {
            std::stringstream textStream(pLayerElement->GetText());
            std::string id;
            while (std::getline(textStream, id, ','))
            {
                m_tiles.push_back(std::stoul(id));
            }
        }
    }
    return true;
}

//-----------------------------------------------------------------------------------------
// TileSetComponent
//-----------------------------------------------------------------------------------------
bool TileSetComponent::Initialize(uint32_t numTiles, uint32_t width, uint32_t height, float ratio)
{
    m_width = width;
    m_height = height;
    m_ratio = ratio;
    m_tiles.reserve(numTiles);
    return true;
}

void TileSetComponent::Render(IGraphics* pGraphics)
{
    const Vector2<int>& camera = ApplicationLayer::GetInstance()->GetGameLayer()->GetCamera().GetRenderingPoint();
    for (auto& tile : m_tiles)
    {
        Rect src = 
        { 
            static_cast<int32_t>(tile.m_srcX), 
            static_cast<int32_t>(tile.m_srcY), 
            static_cast<int32_t>(m_width), 
            static_cast<int32_t>(m_height) 
        };
        
        Rect dest = 
        { 
            static_cast<int32_t>(tile.m_destX * m_ratio),
            static_cast<int32_t>(tile.m_destY * m_ratio),
            static_cast<int32_t>(m_width * m_ratio),
            static_cast<int32_t>(m_height * m_ratio)
        };
        pGraphics->DrawTexture(m_textures.at(tile.m_pName).get(), src, dest);
        //Rect test = { dest.m_x, dest.m_y, 10, 10 };
        //Color test2 = { 255, 0, 0, 0 };
        //pGraphics->RenderFillRect(test, test2);
    }
}

void TileSetComponent::AddTileSet(const char* pName, std::shared_ptr<ITexture2D> pTexture)
{
    m_textures.emplace(pName, pTexture);
}

//-----------------------------------------------------------------------------------------
// Level
//-----------------------------------------------------------------------------------------
// TODO:
//   - [] Add logging when initializing each tile.
std::shared_ptr<Actor> Level::LoadTiles(ResourceCache* pResCache, std::string filePath)
{
    auto pLevel = pResCache->GetHandle(&Resource(filePath));
    if (pLevel == nullptr)
        return nullptr;

    tinyxml2::XMLDocument doc;
    XMLError error = doc.Parse(pLevel->GetData().data(), pLevel->GetData().size());
    if (error != XML_SUCCESS)
    {
        return nullptr;
    }

    XMLElement* pRoot = doc.FirstChildElement();
    if (strcmp(pRoot->Name(), "map") == 0)
    {
        for (XMLElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
        {
            if (strcmp(pElement->Name(), "tileset") == 0)
            {
                // --- Multiple tilesets ---
                TileSet tileData;
                tileData.Initialize(pElement, pResCache, filePath);

                if (!HasTileSet(tileData.GetName().c_str()))
                    m_tileData.emplace(tileData.GetName(), tileData);
            }
            else if (strcmp(pElement->Name(), "layer") == 0)
            {
                Layer layer;
                layer.Initialize(pElement);
                m_layers.push_back(layer);
            }
        }
    }
    else
    {
        return nullptr;
    }

    auto pActor = std::make_shared<Actor>(ApplicationLayer::GetInstance()->GetGameLayer()->GetActorFactory().GetNextActorId());
    size_t tileSize = m_layers[0].GetTiles().size();

    auto pTileComp = std::make_unique<TileSetComponent>(pActor.get(), "TileSetComponent");
    pTileComp->Initialize
    (
        tileSize, 
        m_tileData.begin()->second.GetTileWidth(), 
        m_tileData.begin()->second.GetTileHeight(),
        m_tileData.begin()->second.GetRenderingRatio()
    );

    // Set every tileset to TileSetComp 
    for (auto& it : m_tileData)
    {
        auto tileSet = it.second;
        pTileComp->AddTileSet(tileSet.GetName().c_str(), tileSet.GetTexture());
    }

    for (auto layer : m_layers)
    {
        std::vector<uint32_t>& tiles = layer.GetTiles();
        const std::string& tileSetName = layer.GetUsedTileSet();
        //-------------------------------------------------------------------------------------------------
        // The ID of each tile in the tmx file is expressed in succession.
        // However, there are unique ids of each tile in the tileset.
        // We need to find the unique id used in the tileset to retrieve tile's information.
        //-------------------------------------------------------------------------------------------------
        const uint32_t firstGridId = tileSetName.empty() ? 0 : m_tileData[tileSetName].GetFirstGridId() - 1;

        Vector2<int> cameraPos = Camera.GetRenderingPoint();
        for (size_t tileIndex = 0; tileIndex < tileSize; tileIndex++)
        {
            uint32_t tile = tiles[tileIndex] - firstGridId;
            if (tile > 0) // tile 0 means no tile, so skip this
            {
                TileSetComponent::Tile tileInfo;

                m_tileData[tileSetName].GetSrc(tile, tileInfo.m_srcX, tileInfo.m_srcY);

                int32_t x = tileIndex % layer.GetWidth();
                int32_t y = tileIndex / layer.GetWidth();
                tileInfo.m_destX = x * m_tileData[tileSetName].GetTileWidth();
                tileInfo.m_destY = y * m_tileData[tileSetName].GetTileHeight();
                tileInfo.m_pName = tileSetName;

                pTileComp->AddTile(tileInfo);
            }
        }
    }
    pActor->AddComponent(std::move(pTileComp));
    return pActor;
}

std::vector<std::shared_ptr<Actor>> Level::LoadObjects(ResourceCache* pResCache, std::string filePath)
{
    std::vector<std::shared_ptr<Actor>> actors;

    auto pLevel = pResCache->GetHandle(&Resource(filePath));

    tinyxml2::XMLDocument doc;
    XMLError error = doc.Parse(pLevel->GetData().data(), pLevel->GetData().size());
    if (error != XML_SUCCESS)
    {
        return actors;
    }

    XMLElement* pRoot = doc.FirstChildElement();
    if (strcmp(pRoot->Name(), "map") == 0)
    {
        for (XMLElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement())
        {
            if (strcmp(pElement->Name(), "objectgroup") == 0)
            {
                ParseObjects(pResCache, pElement, actors);
            }
        }
    }
    else
    {
        return actors;
    }

    return actors;
}

bool Level::HasTileSet(const char* pName) const
{
    return m_tileData.find(pName) != m_tileData.end() ? true : false;
}

void Level::ParseObjects(ResourceCache* pResCache, XMLElement* pElement, std::vector<std::shared_ptr<Actor>>& actors)
{
    for (XMLElement* pObjectElement = pElement->FirstChildElement(); pObjectElement;
        pObjectElement = pObjectElement->NextSiblingElement())
    {
        if (strcmp(pObjectElement->Name(), "object") == 0)
        {
            for (XMLElement* pPropsElement = pObjectElement->FirstChildElement(); pPropsElement; pPropsElement = pPropsElement->NextSiblingElement())
            {
                if (strcmp(pPropsElement->Name(), "properties") == 0)
                {
                    ParseObject(pResCache, pObjectElement, pPropsElement, actors);
                }
            }
        }
    }
}

void Level::ParseObject(ResourceCache* pResCache, XMLElement* pObject, XMLElement* pElement, std::vector<std::shared_ptr<Actor>>& actors)
{
    std::string xml;
    std::string name;

    for (XMLElement* pPropElement = pElement->FirstChildElement();
        pPropElement; pPropElement = pPropElement->NextSiblingElement())
    {
        if (strcmp(pPropElement->Name(), "property") == 0)
        {
            if (strcmp(pPropElement->Attribute("name"), "actor") == 0)
            {
                xml = pPropElement->Attribute("value");
            }
            else if (strcmp(pPropElement->Attribute("name"), "objName") == 0)
            {
                name = pPropElement->Attribute("value");
            }
        }
    }

    auto& actorFactory = ApplicationLayer::GetInstance()->GetGameLayer()->GetActorFactory();

    auto pResource = pResCache->GetHandle(&Resource(xml));
    auto pActor = actorFactory.CreateActorByResource(pResource);
    pActor->SetName(name);

    float ratio = m_tileData.begin()->second.GetRenderingRatio();

    auto pTransform = pActor->GetComponent(kTransformId);
    if (pTransform != nullptr)
    {
        TransformComponent* pComponent = static_cast<TransformComponent*>(pTransform);
        pComponent->SetPosition(pObject->FloatAttribute("x") * ratio, pObject->FloatAttribute("y") * ratio);
    }

    auto pStaticBody = pActor->GetComponent(kStaticBodyId);
    if (pStaticBody != nullptr)
    {
        float width = pObject->FloatAttribute("width");
        float height = pObject->FloatAttribute("height");

        if (width > 0 && height > 0)
        {
            IStaticBodyComponent* pComponent = static_cast<IStaticBodyComponent*>(pStaticBody);
            pComponent->SetDimensions((width * ratio), (height * ratio));
        }
    }

    auto pDynamicBody = pActor->GetComponent(kDynamicBodyId);
    if (pDynamicBody != nullptr)
    {
        IDynamicBodyComponent* pComponent = static_cast<IDynamicBodyComponent*>(pDynamicBody);
        pComponent->SetPosition(pObject->FloatAttribute("x") * ratio, pObject->FloatAttribute("y") * ratio);
    }

    actors.push_back(std::move(pActor));
} 