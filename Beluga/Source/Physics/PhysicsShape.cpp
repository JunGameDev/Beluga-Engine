#include <vector>
#include "Physics/Physics.h"

#include "Physics/PhysicsShape.h"
#include "Box2D/box2d.h"

using namespace Bel;
using namespace tinyxml2;

class Box2DPhysicsShape;

class Box2DFixtureDef : public IFixtureDef
{
private:
    b2FixtureDef m_fixtureDef;

public:
    Box2DFixtureDef() {}
    virtual ~Box2DFixtureDef() override {}

    virtual void* GetFixture() override
    {
        return &m_fixtureDef;
    }
};

std::unique_ptr<IFixtureDef> IFixtureDef::Create()
{
    return std::make_unique<Box2DFixtureDef>();
}

class Box2DBoxShape;
class Box2DCircleShape;
class Box2DPolygonShape;
class Box2DChainShape;

/****************************************************************
 *                      Box2DPhysicsShape
 ***************************************************************/
class Box2DPhysicsShape : public IPhysicalShape
{
protected:
    float m_width;
    float m_height;
    float m_density;
    float m_friction;
    float m_radius;

    Type m_type;
    bool m_sensor;
    b2Vec2 m_center;
    std::string m_userData;
    uint16 m_categoryBits;
    uint16 m_maskBits;

public:
    Box2DPhysicsShape()
        : m_width(0)
        , m_height(0)
        , m_density(0)
        , m_radius(0)
        , m_friction(0)
        , m_type(Type::kNone)
        , m_sensor(false)
        , m_center({0, 0})
        , m_categoryBits(1)
        , m_maskBits(1)
    {
    }

    virtual ~Box2DPhysicsShape() override {}
    virtual void Initialize(XMLElement* pData) override {}
    virtual void SetThisToFixture(IFixtureDef* pFixture) override {};

    virtual float GetDensity()  override { return m_density; }
    virtual float GetWidth()    override { return m_width; }
    virtual float GetHeight()   override { return m_height; }
    virtual float GetFriction() override { return m_friction; }
    virtual float GetRadius()   override { return m_radius; }
    virtual Vector2<float> GetCenter() override { return Vector2<float>(m_center.x, m_center.y); }

    static std::unique_ptr<Box2DBoxShape> CreateBoxShape()
    {
        return std::make_unique<Box2DBoxShape>();
    }

    static std::unique_ptr<Box2DCircleShape> CreateCircleShape()
    {
        return std::make_unique<Box2DCircleShape>();
    }

    static std::unique_ptr<Box2DPolygonShape> CreatePolygonShape()
    {
        return std::make_unique<Box2DPolygonShape>();
    }

    static std::unique_ptr<Box2DChainShape> CreateChainShape()
    {
        return std::make_unique<Box2DChainShape>();
    }
};

/****************************************************************
 *                      Box2DBoxShape
 ***************************************************************/
class Box2DBoxShape : public Box2DPhysicsShape
{
private:
    b2PolygonShape m_shape;

public:
    Box2DBoxShape()
        : Box2DPhysicsShape()
    {
    }

    virtual ~Box2DBoxShape() override {}

    virtual void Initialize(XMLElement* pData) override final
    {
        auto pElement = pData->FirstChildElement("Dimensions");
        if (pElement != nullptr)
        {
            m_width = pElement->FloatAttribute("width");
            m_height = pElement->FloatAttribute("height");
        }
        pElement = pData->FirstChildElement("Density");
        if (pElement != nullptr)
        {
            m_density = pElement->FloatText();
        }
        pElement = pData->FirstChildElement("Sensor");
        if (pElement != nullptr)
        {
            m_sensor = pElement->BoolText();
        }
        pElement = pData->FirstChildElement("Center");
        if (pElement != nullptr)
        {
            m_center.x = pElement->FloatAttribute("x") / IPhysicsManager::GetPPM();
            m_center.y = pElement->FloatAttribute("y") / IPhysicsManager::GetPPM();
        }
        pElement = pData->FirstChildElement("UserData");
        if (pElement != nullptr)
        {
            m_userData = pElement->GetText();
        }
        pElement = pData->FirstChildElement("CategoryBits");
        if (pElement != nullptr)
        {
            m_categoryBits = pElement->IntText();
        }
        pElement = pData->FirstChildElement("MaskBits");
        if (pElement != nullptr)
        {
            m_maskBits = pElement->IntText();
        }

        m_shape.SetAsBox(m_width / IPhysicsManager::GetPPM(), m_height / IPhysicsManager::GetPPM(), m_center, 0);
    }

    virtual void SetThisToFixture(IFixtureDef* pFixture) override final
    {
        b2FixtureDef* pFixtureDef = static_cast<b2FixtureDef*>(pFixture->GetFixture());
        pFixtureDef->shape = &m_shape;
        pFixtureDef->density = m_density;
        pFixtureDef->friction = m_friction;
        pFixtureDef->isSensor = m_sensor;
        pFixtureDef->userData = m_userData.data();
        pFixtureDef->filter.categoryBits = m_categoryBits;
        pFixtureDef->filter.maskBits = m_maskBits;
    }
};

/****************************************************************
 *                      Box2DCircleShape
 ***************************************************************/
class Box2DCircleShape : public Box2DPhysicsShape
{
private:
    b2CircleShape m_shape;

public:
    Box2DCircleShape()
        : Box2DPhysicsShape()
    {
    }
    virtual ~Box2DCircleShape() override {}

    virtual void Initialize(XMLElement* pData) override final
    {
        auto pElement = pData->FirstChildElement("Radius");
        if (pElement != nullptr)
        {
            m_radius = pElement->FloatText() / IPhysicsManager::GetPPM();
        }
        pElement = pData->FirstChildElement("Density");
        if (pElement != nullptr)
        {
            m_density = pElement->FloatText();
        }
        pElement = pData->FirstChildElement("Sensor");
        if (pElement != nullptr)
        {
            m_sensor = pElement->BoolText();
        }
        pElement = pData->FirstChildElement("UserData");
        if (pElement != nullptr)
        {
            m_userData = pElement->GetText();
        }
        pElement = pData->FirstChildElement("Friction");
        if (pElement != nullptr)
        {
            m_friction = pElement->FloatText();
        }
        pElement = pData->FirstChildElement("CategoryBits");
        if (pElement != nullptr)
        {
            m_categoryBits = pElement->IntText();
        }
        pElement = pData->FirstChildElement("MaskBits");
        if (pElement != nullptr)
        {
            m_maskBits = pElement->IntText();
        }
    
        m_shape.m_type = b2Shape::Type::e_circle;
        m_shape.m_radius = m_radius;
    }

    virtual void SetThisToFixture(IFixtureDef* pFixture) override final
    {
        b2FixtureDef* pFixtureDef = static_cast<b2FixtureDef*>(pFixture->GetFixture());
        pFixtureDef->shape = &m_shape;
        pFixtureDef->density = m_density;
        pFixtureDef->friction = m_friction;
        pFixtureDef->isSensor = m_sensor;
        pFixtureDef->userData = m_userData.data();

        pFixtureDef->filter.categoryBits = m_categoryBits;
        pFixtureDef->filter.maskBits = m_maskBits;
    }
};

/****************************************************************
 *                      Box2DPolygonShape
 ***************************************************************/
class Box2DPolygonShape : public Box2DPhysicsShape
{
private:
    b2PolygonShape m_shape;

public:
    Box2DPolygonShape()
    {
    }
    virtual ~Box2DPolygonShape() override {}

    virtual void Initialize(XMLElement* pData) override final
    {
        auto pElement = pData->FirstChildElement("Radius");
        if (pElement != nullptr)
        {
            m_radius = pElement->FloatAttribute("value");
        }
        pElement = pData->FirstChildElement("Density");
        if (pElement != nullptr)
        {
            m_density = pElement->FloatText();
        }
        pElement = pData->FirstChildElement("Vertices");
        if (pElement != nullptr)
        {
            std::vector<b2Vec2> vertices;
            for (XMLElement* pVertexElem = pElement->FirstChildElement(); pVertexElem; pVertexElem = pVertexElem->NextSiblingElement())
            {
                vertices.emplace_back(pVertexElem->FloatAttribute("x") / IPhysicsManager::GetPPM(), pVertexElem->FloatAttribute("y") / IPhysicsManager::GetPPM());
            }
            
            b2Vec2* pVertices = &vertices[0];
            m_shape.Set(pVertices, vertices.size());
        }
        pElement = pData->FirstChildElement("CategoryBits");
        if (pElement != nullptr)
        {
            m_categoryBits = pElement->IntText();
        }
        pElement = pData->FirstChildElement("MaskBits");
        if (pElement != nullptr)
        {
            m_maskBits = pElement->IntText();
        }

        m_shape.m_type = b2Shape::Type::e_polygon;
        m_shape.m_radius = m_radius;
    }

    virtual void SetThisToFixture(IFixtureDef* pFixture) override final
    {
        b2FixtureDef* pFixtureDef = static_cast<b2FixtureDef*>(pFixture->GetFixture());
        pFixtureDef->shape = &m_shape;
        pFixtureDef->density = m_density;
        pFixtureDef->filter.categoryBits = m_categoryBits;
        pFixtureDef->filter.maskBits = m_maskBits;
    }
};

/****************************************************************
 *                      Box2DChainShape
 ***************************************************************/
class Box2DChainShape : public Box2DPhysicsShape
{
private:
    b2ChainShape m_shape;
    bool m_loop;

public:
    Box2DChainShape()
        : m_shape()
        , m_loop(false)
    {
    }
    virtual ~Box2DChainShape() override {}

    virtual void Initialize(XMLElement* pData) override final
    {
        auto pElement = pData->FirstChildElement("Radius");
        if (pElement != nullptr)
        {
            m_radius = pElement->FloatAttribute("value");
        }
        pElement = pData->FirstChildElement("Density");
        if (pElement != nullptr)
        {
            m_density = pElement->FloatText();
        }
        pElement = pData->FirstChildElement("Vertices");

        std::vector<b2Vec2> vertices;
        if (pElement != nullptr)
        {
            for (XMLElement* pVertexElem = pElement->FirstChildElement(); pVertexElem; pVertexElem = pVertexElem->NextSiblingElement())
            {
                vertices.emplace_back(pVertexElem->FloatAttribute("x") / IPhysicsManager::GetPPM(), pVertexElem->FloatAttribute("y") / IPhysicsManager::GetPPM());
            }
        }

        pElement = pData->FirstChildElement("Loop");
        if (pElement != nullptr)
        {
            const char* pTxt = pElement->GetText();
            if (pTxt == nullptr)
                return;

            if (std::strcmp(pTxt, "true") == 0 || std::strcmp(pTxt, "True") == 0 || std::strcmp(pTxt, "TRUE") == 0 || std::strcmp(pTxt, "1") == 0)
            {
                m_loop = true;
            }
        }
        pElement = pData->FirstChildElement("CategoryBits");
        if (pElement != nullptr)
        {
            m_categoryBits = pElement->IntText();
        }
        pElement = pData->FirstChildElement("MaskBits");
        if (pElement != nullptr)
        {
            m_maskBits = pElement->IntText();
        }

        b2Vec2* pVertices = &vertices[0];

        if (pVertices == nullptr)
            return;

        if (!m_loop)
        {
            m_shape.CreateChain(pVertices, vertices.size());
        }
        else
        {
            m_shape.CreateLoop(pVertices, vertices.size());
        }

        m_shape.m_type = b2Shape::Type::e_polygon;
    }

    virtual void SetThisToFixture(IFixtureDef* pFixture) override final
    {
        b2FixtureDef* pFixtureDef = static_cast<b2FixtureDef*>(pFixture->GetFixture());
        pFixtureDef->shape = &m_shape;
        pFixtureDef->density = m_density;
        pFixtureDef->filter.categoryBits = m_categoryBits;
        pFixtureDef->filter.maskBits = m_maskBits;

    }
};

std::unique_ptr<IPhysicalShape> IPhysicalShape::Create(Type& type)
{
    switch (type)
    {
    case Bel::IPhysicalShape::Type::kBox:
        return Box2DPhysicsShape::CreateBoxShape();
        break;
    case Bel::IPhysicalShape::Type::kCircle:
        return Box2DPhysicsShape::CreateCircleShape();
        break;
    case Bel::IPhysicalShape::Type::kPolygon:
        return Box2DPhysicsShape::CreatePolygonShape();
        break;
    case Bel::IPhysicalShape::Type::kChain:
        return Box2DPhysicsShape::CreateChainShape();
        break;
    default:
        break;
    }

    return nullptr;
}