#include "Physics/Physics.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Graphics/Graphics.h"
#include "Box2D/box2d.h"
#include "Input/Input.h"
using namespace Bel;

namespace Lua 
{
    static int TransformComponentMove(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));

        // Converting to float from Lua's double
        float x = static_cast<float>(luaL_checknumber(pState, 2));
        float y = static_cast<float>(luaL_checknumber(pState, 3));
        pTransform->Move(x, y);

        // Wiping out the stack.
        lua_pop(pState, 3);

        return 0;
    }

    static int TransformComponentMoveConstantly(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));

        // Converting to float from Lua's double
        float x = static_cast<float>(luaL_checknumber(pState, 2));
        float y = static_cast<float>(luaL_checknumber(pState, 3));
        pTransform->MoveConstantly(x, y);

        // Wiping out the stack.
        lua_pop(pState, 3);

        return 0;
    }

    static int TransformComponentSetPosition(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));

        // Converting to float from Lua's double
        float x = static_cast<float>(luaL_checknumber(pState, 2));
        float y = static_cast<float>(luaL_checknumber(pState, 3));
        pTransform->SetPosition(x, y);

        // Wiping out the stack.
        lua_pop(pState, 3);

        return 0;
    }

    static int TranformComponentGetX(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));
        lua_pop(pState, 1);
        lua_pushnumber(pState, pTransform->GetPosition().m_x); // Push the x pos;
        return 1;
    }

    static int TranformComponentGetY(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));
        lua_pop(pState, 1);
        lua_pushnumber(pState, pTransform->GetPosition().m_y); // Push the y pos;
        return 1;
    }

    static int TransformComponentGetPosition(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));
        lua_pop(pState, 1);
        lua_pushnumber(pState, pTransform->GetPosition().m_x); // Push the x pos;
        lua_pushnumber(pState, pTransform->GetPosition().m_y); // Push the y pos;
        return 2;
    }

    static int TransformComponentSetSpeed(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));
        int speed = static_cast<int>(lua_tointeger(pState, 2));
        lua_pop(pState, 2);
        
        pTransform->SetSpeed(speed);
        return 0;
    }

    static int TranformComponentGetSpeed(lua_State* pState)
    {
        auto pTransform = reinterpret_cast<TransformComponent*>(lua_touserdata(pState, 1));
        lua_pop(pState, 1);
        lua_pushnumber(pState, pTransform->GetSpeed()); // Push the speed
        return 1;
    }

    static int DynamicBodyApplyLinearImpulse(lua_State* pState)
    {
        auto pBody = reinterpret_cast<IDynamicBodyComponent*>(lua_touserdata(pState, 1));
        float xVel = static_cast<float>(luaL_checknumber(pState, 2));
        float yVel = static_cast<float>(luaL_checknumber(pState, 3));
        
        pBody->ApplyLinearImpulse(xVel, yVel);
        lua_pop(pState, 3);

        return 0;
    }

    static int DynamicBodySetPosition(lua_State* pState)
    {
        auto pBody = reinterpret_cast<IDynamicBodyComponent*>(lua_touserdata(pState, 1));
        float x = static_cast<float>(luaL_checknumber(pState, 2));
        float y = static_cast<float>(luaL_checknumber(pState, 3));

        pBody->SetPosition(x, y);
        lua_pop(pState, 3);

        return 0;
    }

    static int DynamicBodyReDimension(lua_State* pState)
    {
        auto pBody = reinterpret_cast<IDynamicBodyComponent*>(lua_touserdata(pState, 1));
        float width = static_cast<float>(luaL_checknumber(pState, 2));
        float height = static_cast<float>(luaL_checknumber(pState, 3));
        const char* pUserData = static_cast<const char*>(lua_tostring(pState, 4));
        float centerX = static_cast<float>(luaL_checknumber(pState, 5));
        float centerY = static_cast<float>(luaL_checknumber(pState, 6));
        lua_pop(pState, 6);

        pBody->ReDimension(width, height, pUserData, Vector2<float>(centerX, centerY));

        return 0;
    }
}

/****************************************************************
 *                      TransformComponent
 ***************************************************************/
bool TransformComponent::Initialize(tinyxml2::XMLElement* pData)
{
    auto pElement = pData->FirstChildElement("Position");
    if (pElement != nullptr)
    {
        m_position.Set(pElement->FloatAttribute("x"), pElement->FloatAttribute("y"));
    }

    pElement = pData->FirstChildElement("Movement");
    if (pElement != nullptr)
    {
        m_speed = pElement->IntAttribute("speed");
    }

    return true;
}

void TransformComponent::SetDegree(float degree)
{
    m_degree = degree;
    m_radian = (b2_pi / 180.f) * degree;
}

void TransformComponent::SetRadian(float radian)
{
    m_radian = radian;
    m_degree = (180.f / b2_pi) * radian;
}

void TransformComponent::RegisterWithScript()
{
    auto& scripting = ApplicationLayer::GetInstance()->GetGameLayer()->GetScriptingManager();
    scripting.GetGlobal("g_logic");
    scripting.GetFromTable("actors");
    scripting.GetFromTable(GetOwner()->GetId());
    
    // Used to represent our component.
    scripting.CreateTable();
    scripting.AddToTable("this", this); // Add 'this' key to it, then assign pointer.
    scripting.AddToTable("Move", Lua::TransformComponentMove);
    scripting.AddToTable("MoveConstantly", Lua::TransformComponentMoveConstantly);
    scripting.AddToTable("SetPosition", Lua::TransformComponentSetPosition);
    scripting.AddToTable("SetSpeed", Lua::TransformComponentSetSpeed);
    scripting.AddToTable("GetSpeed", Lua::TranformComponentGetSpeed);

    scripting.CreateTable();
    scripting.AddToTable("GetX", Lua::TranformComponentGetX);
    scripting.AddToTable("GetY", Lua::TranformComponentGetY);
    scripting.AddToTable("GetPosition", Lua::TransformComponentGetPosition);
    scripting.AddToTable("Position");

    scripting.AddToTable("TransformComponent");


    scripting.PopAll();
}

/****************************************************************
 *                      Box2DStaticBody
 ***************************************************************/
class Box2DStaticBody : public IStaticBodyComponent
{
private:
    b2World*    m_pParentWorld;
    b2Body*     m_pBody;
    b2Fixture*  m_pFixture;

    TransformComponent* m_pTransform;
    float m_width;
    float m_height;
    bool m_isSensor;
    uint16_t m_categoryBits;
    uint16_t m_maskBits;

public:
    Box2DStaticBody(Actor* pOwner, std::string_view name, b2World* pWorld)
        : IStaticBodyComponent(pOwner, name)
        , m_pParentWorld(pWorld)
        , m_pBody(nullptr)
        , m_pFixture(nullptr)
        , m_pTransform(nullptr)
        , m_width(0)
        , m_height(0)
        , m_isSensor(false)
        , m_categoryBits(1)
        , m_maskBits(1)
    {
    }

    virtual ~Box2DStaticBody()
    {
        auto game = ApplicationLayer::GetInstance()->GetGameLayer();
        if (game != nullptr)
        {
            game->GetPhysicsManager().UnregisterStaticBody(this);
        }

        if (m_pFixture != nullptr)
        {
            m_pBody->DestroyFixture(m_pFixture);
        }
        if (m_pBody != nullptr)
        {
            m_pParentWorld->DestroyBody(m_pBody);
        }
    }

    const b2Fixture* GetFixture() { return m_pFixture; }

    virtual bool Initialize(tinyxml2::XMLElement* pData) override
    {
        auto pElement = pData->FirstChildElement("Dimensions");
        if (pElement != nullptr)
        {
            m_width = pElement->FloatAttribute("width");
            m_height = pElement->FloatAttribute("height");
        }
        pElement = pData->FirstChildElement("Sensor");
        if (pElement != nullptr)
        {
            m_isSensor = pElement->BoolText();
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

        return true;
    }

    virtual bool PostInit() override
    {
        m_pTransform = static_cast<TransformComponent*>(GetOwner()->GetComponent(kTransformId));
        if (m_pTransform == nullptr)
        {
            auto& log = ApplicationLayer::GetInstance()->GetLogging();
            log.Log(Logging::SeverityLevel::kLevelError, "Unable to find TransformComponent on Actor");
            return false;
        }

        // Create a definition for the body.
        b2BodyDef body;
        body.type = b2_staticBody;
        auto& position = m_pTransform->GetPosition();
        //body.position.Set(position.m_x / IPhysicsManager::GetPPM(), position.m_y / IPhysicsManager::GetPPM());
        body.position.Set(position.m_x - (m_width / 2), position.m_y + (m_height / 2));
        m_pBody = m_pParentWorld->CreateBody(&body);

        
        // Define out shape.
        b2PolygonShape shape;
        shape.SetAsBox(m_width / 2.0f /*/ IPhysicsManager::GetPPM()*/, m_height / 2.0f /*/ IPhysicsManager::GetPPM()*/);

        // Create a fixture on our body to anchor it into the physics world.
        b2FixtureDef fixture;
        fixture.shape = &shape;
        fixture.density = 1000.f;
        fixture.userData = this;
        fixture.isSensor = m_isSensor;
        fixture.filter.categoryBits = m_categoryBits;
        fixture.filter.maskBits = m_maskBits;

        m_pFixture = m_pBody->CreateFixture(&fixture);
        ApplicationLayer::GetInstance()->GetGameLayer()->GetPhysicsManager().RegisterStaticBody(this);
        return true;
    }

    virtual void Update(float delta) override
    {
        auto& pos = m_pBody->GetPosition();
        m_pBody->SetTransform(b2Vec2(
            ((m_pTransform->GetPosition().m_x + (m_width / 2.f)) / IPhysicsManager::GetPPM()),
            ((m_pTransform->GetPosition().m_y + (m_height / 2.f)) / IPhysicsManager::GetPPM())),
            0);
    }

    virtual void SetDimensions(float width, float height) override
    {
        b2PolygonShape* pShape = static_cast<b2PolygonShape*>(m_pFixture->GetShape());
        m_width = width;
        m_height = height;
        pShape->SetAsBox(m_width / 2.0f / IPhysicsManager::GetPPM(), m_height / 2.0f / IPhysicsManager::GetPPM());
    }

    virtual void SetPosition(float x, float y) override
    {
        m_pBody->SetTransform(b2Vec2(x, y), m_pBody->GetAngle());
    }
};

/****************************************************************
 *                      Box2DDynamicBody
 ***************************************************************/
//-----------------------------------------------------------------------------------------
// Box2DDynamicBody
// 
// [ Description ] 
//
// TODO: 
//-----------------------------------------------------------------------------------------
class Box2DDynamicBody : public IDynamicBodyComponent
{
private:
    b2World* m_pParentWorld;
    b2Body* m_pBody;
    //b2Fixture* m_pFixture;

    TransformComponent* m_pTransform;
    bool m_fixedRotation;
    bool m_isBullet;

    float m_width;
    float m_height;
    float m_gravityScale;
    float m_damping;

    std::unique_ptr<IPhysicalShape> m_pShape;
    std::vector<b2Fixture*> m_fixtures;
    std::vector<std::unique_ptr<IPhysicalShape>> m_shapes;


public:
    Box2DDynamicBody(Actor* pOwner, const char* pName, b2World* pWorld)
        : IDynamicBodyComponent(pOwner, pName)
        , m_pParentWorld(pWorld)
        , m_pBody(nullptr)
        , m_pTransform(nullptr)
        , m_fixedRotation(false)
        , m_isBullet(false)
        , m_width(0)
        , m_height(0)
        , m_gravityScale(1)
        , m_damping(0)
    {
    }

    virtual ~Box2DDynamicBody()
    {

        auto game = ApplicationLayer::GetInstance()->GetGameLayer();
        if (game != nullptr)
        {
            game->GetPhysicsManager().UnregisterDynamicBody(this);
        }
        if (m_pBody != nullptr)
        {
            m_pParentWorld->DestroyBody(m_pBody);
        }
    }
    
    // other functions
    virtual bool Initialize(tinyxml2::XMLElement* pData) override
    {
        auto pElement = pData->FirstChildElement("Dimensions");
        if (pElement != nullptr)
        {
            m_width = pElement->FloatAttribute("width");
            m_height = pElement->FloatAttribute("height");
        }

        pElement = pData->FirstChildElement("Damping");
        if (pElement != nullptr)
        {
            m_damping = pElement->FloatText();
        }

        pElement = pData->FirstChildElement("Fixed");
        if (pElement != nullptr)
        {
            m_fixedRotation = pElement->BoolText();
        }

        pElement = pData->FirstChildElement("Bullet");
        if (pElement != nullptr)
        {
            m_isBullet = pElement->BoolText();
        }

        pElement = pData->FirstChildElement("GravityScale");
        if (pElement != nullptr)
        {
            m_gravityScale = pElement->FloatText();
        }

        for (tinyxml2::XMLElement* pFixtureData = pData->FirstChildElement(); pFixtureData; pFixtureData = pFixtureData->NextSiblingElement())
        {
            pElement = pFixtureData->FirstChildElement("Type");
            if (pElement != nullptr)
            {
                const char* pTxt = pElement->GetText();
                IPhysicalShape::Type type;

                if (std::strcmp(pTxt, "Box") == 0)
                {
                    type = IPhysicalShape::Type::kBox;
                }
                else if (std::strcmp(pTxt, "Circle") == 0)
                {
                    type = IPhysicalShape::Type::kCircle;
                }
                else if (std::strcmp(pTxt, "Polygon") == 0)
                {
                    type = IPhysicalShape::Type::kPolygon;
                }
                else if (std::strcmp(pTxt, "Chain") == 0)
                { 
                    type = IPhysicalShape::Type::kChain;
                }

                auto pShape = IPhysicalShape::Create(type);
                if (pShape == nullptr)
                {
                    LOG("Failed to create Physical shape for dynamic body");
                    return false;
                }

                pShape->Initialize(pFixtureData);
                m_shapes.emplace_back(std::move(pShape));
            }
        }

        return true;
    }

    virtual bool PostInit() override
    {
        m_pTransform = static_cast<TransformComponent*>(GetOwner()->GetComponent(kTransformId));
        if (m_pTransform == nullptr)
        {
            auto& log = ApplicationLayer::GetInstance()->GetLogging();
            log.Log(Logging::SeverityLevel::kLevelError, "Unable to find TransformComponent on Actor");
            return false;
        }

        // Create a definition for the body.
        b2BodyDef body;
        body.type = b2_dynamicBody;
        auto& position = m_pTransform->GetPosition();
        body.position.Set
        (
            position.m_x / IPhysicsManager::GetPPM(), 
            position.m_y / IPhysicsManager::GetPPM()
        );
        m_pBody = m_pParentWorld->CreateBody(&body);
        m_pBody->SetFixedRotation(m_fixedRotation);
        m_pBody->SetLinearDamping(m_damping);
        m_pBody->SetGravityScale(m_gravityScale);
        m_pBody->SetBullet(m_isBullet);

        // Set fixture
        for (uint32_t i = 0; i < m_shapes.size(); ++i)
        {
            std::unique_ptr<IFixtureDef> pFixture = IFixtureDef::Create();
            m_shapes[i]->SetThisToFixture(pFixture.get());
            m_fixtures.emplace_back(m_pBody->CreateFixture(static_cast<b2FixtureDef*>(pFixture->GetFixture())));
        }

        ApplicationLayer::GetInstance()->GetGameLayer()->GetPhysicsManager().RegisterDynamicBody(this);

        return true;
    }

    virtual void Update(float delta) override
    {
        auto& pos = m_pBody->GetPosition();
        
        m_pTransform->SetPosition((pos.x * IPhysicsManager::GetPPM()) - (m_width / 2.0f),
                                  (pos.y * IPhysicsManager::GetPPM()) - (m_height / 2.0f));
        
        m_pTransform->SetRadian(m_pBody->GetAngle());
    }

    virtual Vector2<float> GetLinearVelocity() override
    {
        b2Vec2 vec = m_pBody->GetLinearVelocity();
        return Vector2<float>(vec.x * IPhysicsManager::GetPPM(), vec.y * IPhysicsManager::GetPPM());
    }

    virtual float GetAngularVelocity() override
    {
        return m_pBody->GetAngularVelocity();
    }

    virtual void SetLinearVelocity(float x, float y) override
    {
        m_pBody->SetLinearVelocity(b2Vec2(IPhysicsManager::ConvertMeterToPixel(x), IPhysicsManager::ConvertMeterToPixel(y)));
    }

    virtual void SetLinearVelocity(Vector2<float> vec) override
    {
        SetLinearVelocity(vec.m_x / IPhysicsManager::GetPPM(), vec.m_y / IPhysicsManager::GetPPM());
    }

    virtual void SetLinearVelocityX(float x) override
    {
        b2Vec2 vec = m_pBody->GetLinearVelocity();
        vec.x = x;
        m_pBody->SetLinearVelocity(vec);
    }

    virtual void SetLinearVelocityY(float y) override
    {
        b2Vec2 vec = m_pBody->GetLinearVelocity();
        vec.y = y;
        m_pBody->SetLinearVelocity(vec);
    }

    virtual void ApplyLinearImpulse(float x, float y) override
    {
        m_pBody->ApplyLinearImpulse(b2Vec2(x, y), m_pBody->GetWorldCenter(), true);
    }

    virtual void ApplyForce(float x, float y) override
    {
        m_pBody->ApplyForce(b2Vec2(x, y), m_pBody->GetWorldCenter(), true);
    }

    virtual void ApplyForceToCenter(float x, float y, bool awake) override
    {
        m_pBody->ApplyForceToCenter(b2Vec2(x, y), awake);
    }

    virtual void ApplyTorque(float val) override
    {
        m_pBody->ApplyTorque(val, true);
    }

    virtual void SetPosition(float x, float y) override
    {
        m_pBody->SetTransform(b2Vec2(x / IPhysicsManager::GetPPM(), y / IPhysicsManager::GetPPM()), m_pBody->GetAngle());
    }

    virtual void SetRadian(float rad) override
    {
        m_pTransform->SetRadian(rad);
    }

    virtual float GetMass() override
    {
        return m_pBody->GetMass();
    }

    virtual void RegisterWithScript() override
    {
        auto& scripting = ApplicationLayer::GetInstance()->GetGameLayer()->GetScriptingManager();
        scripting.GetGlobal("g_logic");
        scripting.GetFromTable("actors");
        scripting.GetFromTable(GetOwner()->GetId());

        // Used to represent our component.
        scripting.CreateTable();
        scripting.AddToTable("this", this); // Add 'this' key to it, then assign pointer.
        scripting.AddToTable("ApplyLinearImpulse", Lua::DynamicBodyApplyLinearImpulse);
        scripting.AddToTable("SetPosition", Lua::DynamicBodySetPosition);
        scripting.AddToTable("ReDimension", Lua::DynamicBodyReDimension);

        scripting.AddToTable("DynamicBodyComponent");

        scripting.PopAll();
    }

    virtual void ReDimension(float width, float height, const char* pUserData, Vector2<float> center) override
    {
        auto pFixtureList = m_pBody->GetFixtureList();
        
        for (auto pFixture = pFixtureList; pFixture; pFixture = pFixture->GetNext())
        {
            if (!pFixture)
                continue;

            const char* pFixtureUserData = static_cast<const char*>(pFixture->GetUserData());
            if (pFixtureUserData == nullptr)
                continue;

            if (std::strcmp(pFixtureUserData, pUserData) == 0)
            {
                b2PolygonShape* pShape = static_cast<b2PolygonShape*>(pFixture->GetShape());
                m_width = width;
                m_height = height;
                b2Vec2 b2Center = { center.m_x / IPhysicsManager::GetPPM(), center.m_y / IPhysicsManager::GetPPM() };
                pShape->SetAsBox(m_width / IPhysicsManager::GetPPM(), m_height / IPhysicsManager::GetPPM(), b2Center, 0);
            }
        }
    }
    

    const b2Fixture* GetFixture() { return m_pBody->GetFixtureList(); }
};

/****************************************************************
 *                      Box2DContactListener
 ***************************************************************/
 //-----------------------------------------------------------------------------------------
 // Box2DDynamicBody
 // 
 // [ Description ] 
 //     Stores all the contacts that started and ended
 //
 // TODO: 
 //-----------------------------------------------------------------------------------------
class Box2DContactListener : public b2ContactListener
{
public:
    struct Contact
    {
        b2Fixture* m_pFixtureA;
        b2Fixture* m_pFixtureB;
    };

private:
    std::vector<Contact> m_beginContact;
    std::vector<Contact> m_endContact;

public:

    virtual ~Box2DContactListener() {}

    virtual void BeginContact(b2Contact* pContact) override
    {
        Contact contact;
        contact.m_pFixtureA = pContact->GetFixtureA();
        contact.m_pFixtureB = pContact->GetFixtureB();
        m_beginContact.push_back(contact);
    }

    virtual void EndContact(b2Contact* pContact) override
    {
        Contact contact;
        contact.m_pFixtureA = pContact->GetFixtureA();
        contact.m_pFixtureB = pContact->GetFixtureB();
        m_endContact.push_back(contact);
    }

    void ClearResults()
    {
        m_beginContact.clear();
        m_endContact.clear();
    }

    std::vector<Contact>& GetBeginContacts() { return m_beginContact; }
    std::vector<Contact>& GetEndContacts()   { return m_endContact;   }
};

/****************************************************************
 *                      Box2DDebugDraw
 ***************************************************************/
class Box2DDebugDraw : public b2Draw
{
public:
    Box2DDebugDraw()
    {
    }

    ~Box2DDebugDraw()
    {

    }

    // Inherited via b2Draw
    virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        Vector2<int> camera = Camera.GetRenderingPoint();

        Point* pPoints = new Point[vertexCount];

        for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            pPoints[vertexIndex].m_x = static_cast<int>(IPhysicsManager::ConvertMeterToPixel(vertices[vertexIndex].x) - camera.m_x);
            pPoints[vertexIndex].m_y = static_cast<int>(IPhysicsManager::ConvertMeterToPixel(vertices[vertexIndex].y) - camera.m_y);
        }

        Color lineColor{ 255, 0, 0, 255 };
        auto pGraphics = ApplicationLayer::GetInstance()->GetGraphics();

        int lastIndex = vertexCount - 1;
        for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            if (vertexIndex != lastIndex)
            {
                pGraphics->RenderLine(pPoints[vertexIndex], pPoints[vertexIndex + 1], lineColor);
            }
            else
            {
                pGraphics->RenderLine(pPoints[vertexIndex], pPoints[0], lineColor);
            }
        }
        
        delete[] pPoints;
    }
    virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        DrawPolygon(vertices, vertexCount, color);
    }
    virtual void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override
    {
        Vector2<int> camera = Camera.GetRenderingPoint();
        
        auto pGraphics = ApplicationLayer::GetInstance()->GetGraphics();
        pGraphics->RenderCircle
        (
            (static_cast<int>(IPhysicsManager::ConvertMeterToPixel(center.x)) - camera.m_x), 
            (static_cast<int>(IPhysicsManager::ConvertMeterToPixel(center.y)) - camera.m_y), 
            IPhysicsManager::ConvertMeterToPixel(radius)
        );
    }
    virtual void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override
    {
        DrawCircle(center, radius, color);
    }
    virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
    {
        auto pGraphics = ApplicationLayer::GetInstance()->GetGraphics();
        Vector2<int> camera = Camera.GetRenderingPoint();

        Point point2 = 
        { 
            static_cast<int>(IPhysicsManager::ConvertMeterToPixel(p2.x - static_cast<float>(camera.m_x))), 
            static_cast<int>(IPhysicsManager::ConvertMeterToPixel(p2.y - static_cast<float>(camera.m_y)))
        };

        Point point1 = 
        { 
            static_cast<int>(IPhysicsManager::ConvertMeterToPixel(p1.x - static_cast<float>(camera.m_x))), 
            static_cast<int>(IPhysicsManager::ConvertMeterToPixel(p1.y - static_cast<float>(camera.m_y)))
        };
        Color col = { static_cast<uint8_t>(color.r), static_cast<uint8_t>(color.g), static_cast<uint8_t>(color.b), static_cast<uint8_t>(color.a) };
        pGraphics->RenderLine(point1, point2, col);
    }
    virtual void DrawTransform(const b2Transform& xf) override
    {
    }
    virtual void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override
    {
    }
};


/****************************************************************
 *                      Box2DPhysics
 ***************************************************************/
class Box2DPhysics : public IPhysicsManager
{
public:
    static const int32_t VELOCITY_ITERATIONS = 8;
    static const int32_t POSITION_ITERATIONS = 3;

private:
    b2World m_world;
    std::unique_ptr<Box2DContactListener> m_pBox2DContactListener;
    std::unordered_map<const b2Fixture*, Actor*> m_fixtureToActor;
    std::shared_ptr<ContactListener> m_pContactListener;
    Box2DDebugDraw m_debugDraw;

public:

    Box2DPhysics(float xGravity, float yGravity)
        : m_world(b2Vec2(xGravity, yGravity))
    {
    }

    virtual bool Initialize() override
    {
        m_debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit | b2Draw::e_pairBit);
        m_world.SetDebugDraw(&m_debugDraw);

        m_pBox2DContactListener = std::make_unique<Box2DContactListener>();
        if (m_pBox2DContactListener == nullptr)
            return false;

        m_world.SetContactListener(m_pBox2DContactListener.get());
        return true;
    }

    virtual void Update(float delta) override
    {
        m_world.Step(delta, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

        if (m_pContactListener)
        {
            auto& beginContacts = m_pBox2DContactListener->GetBeginContacts();
            for (auto& contact : beginContacts)
            {
                auto pActorA = m_fixtureToActor[contact.m_pFixtureA];
                auto pActorB = m_fixtureToActor[contact.m_pFixtureB];

                auto pBody = contact.m_pFixtureA->GetBody()->GetType();
                if (pActorA == nullptr || pActorB == nullptr)
                    continue;

                IActorComponent* pColliderA = nullptr;
                IActorComponent* pColliderB = nullptr;

                switch (contact.m_pFixtureA->GetBody()->GetType())
                {
                case b2BodyType::b2_staticBody:
                    pColliderA = static_cast<IStaticBodyComponent*>(pActorA->GetComponent(kStaticBodyId));
                    break;
                case b2BodyType::b2_dynamicBody:
                    pColliderA = static_cast<IDynamicBodyComponent*>(pActorA->GetComponent(kDynamicBodyId));
                    break;
                default:
                    break;
                }

                switch (contact.m_pFixtureB->GetBody()->GetType())
                {
                case b2BodyType::b2_staticBody:
                    pColliderB = static_cast<IStaticBodyComponent*>(pActorB->GetComponent(kStaticBodyId));
                    break;
                case b2BodyType::b2_dynamicBody:
                    pColliderB = static_cast<IDynamicBodyComponent*>(pActorB->GetComponent(kDynamicBodyId));
                    break;
                default:
                    break;
                }

                // I'm only gonna deal with collision between two dynamic bodies.
                // Let me know if you have any better idea.
                if (pColliderA == nullptr || pColliderB == nullptr)
                    continue;
                
                Collision colA = { pActorA, pColliderA, std::string(static_cast<const char*>(contact.m_pFixtureB->GetUserData())) };
                Collision colB = { pActorB, pColliderB, std::string(static_cast<const char*>(contact.m_pFixtureA->GetUserData())) };

                CollisionGroup group = { colA, colB };
                if (contact.m_pFixtureA->IsSensor() || contact.m_pFixtureB->IsSensor())
                {
                    m_pContactListener->BeginActorTrigger(group);
                }
                else
                {
                    m_pContactListener->BeginActorCollision(group);
                }
            }

            auto& endContacts = m_pBox2DContactListener->GetEndContacts();
            for (auto& contact : endContacts)
            {
                auto pActorA = m_fixtureToActor[contact.m_pFixtureA];
                auto pActorB = m_fixtureToActor[contact.m_pFixtureB];

                if (pActorA == nullptr || pActorB == nullptr)
                    continue;

                auto pColliderA = static_cast<IDynamicBodyComponent*>(pActorA->GetComponent(kDynamicBodyId));
                auto pColliderB = static_cast<IDynamicBodyComponent*>(pActorB->GetComponent(kDynamicBodyId));

                if (pColliderA == nullptr || pColliderB == nullptr)
                    continue;

                Collision colA = { pActorA, pColliderA };
                Collision colB = { pActorB, pColliderB };

                CollisionGroup group = { colA, colB };

                if (contact.m_pFixtureA->IsSensor() || contact.m_pFixtureB->IsSensor())
                {
                    m_pContactListener->EndActorTrigger(group);
                }
                else
                {
                    m_pContactListener->EndActorCollision(group);
                }
            }
        }
        m_pBox2DContactListener->ClearResults();
        m_world.ClearForces();
    }

    virtual std::unique_ptr<IStaticBodyComponent> CreateStaticBodyComponent(Actor* pOwner, const char* pName) override
    {
        return std::make_unique<Box2DStaticBody>(pOwner, pName, &m_world);
    }

    virtual std::unique_ptr<IDynamicBodyComponent> CreateDynamicBodyComponent(Actor* pOwner, const char* pName) override
    {
        return std::make_unique<Box2DDynamicBody>(pOwner, pName, &m_world);
    }

    // other functions
    virtual void RegisterStaticBody(IStaticBodyComponent* pComponent) override
    {
        auto pBox2DComponent = static_cast<Box2DStaticBody*>(pComponent);
        m_fixtureToActor[pBox2DComponent->GetFixture()] = pBox2DComponent->GetOwner();
    }
    virtual void UnregisterStaticBody(IStaticBodyComponent* pComponent) override
    {
        auto pBox2DComponent = static_cast<Box2DStaticBody*>(pComponent);
        m_fixtureToActor.erase(pBox2DComponent->GetFixture());
    }
    virtual void RegisterDynamicBody(IDynamicBodyComponent* pComponent) override
    {
        auto pBox2DComponent = static_cast<Box2DDynamicBody*>(pComponent);
        for (auto pFixture = pBox2DComponent->GetFixture(); pFixture; pFixture = pFixture->GetNext())
        {
            m_fixtureToActor[pFixture] = pBox2DComponent->GetOwner();
        }
    }
    virtual void UnregisterDynamicBody(IDynamicBodyComponent* pComponent) override
    {
        auto pBox2DComponent = static_cast<Box2DDynamicBody*>(pComponent);
        for (auto pFixture = pBox2DComponent->GetFixture(); pFixture; pFixture = pFixture->GetNext())
        {
            m_fixtureToActor.erase(pFixture);
        }
    }

    virtual float GetGravity() override
    {
        return static_cast<float>(m_world.GetGravity().Normalize());
    }

    virtual void SetContactListener(std::shared_ptr<ContactListener> pContactListener) override
    {
        m_pContactListener = pContactListener;
    }

    virtual void DrawDebugLine() override
    {
        m_world.DebugDraw();
    }
};

std::unique_ptr<IPhysicsManager> IPhysicsManager::Create(float xGravity, float yGravity)
{
    return std::make_unique<Box2DPhysics>(xGravity, yGravity);
}

void ContactListener::BeginActorCollision(CollisionGroup& group)
{
    auto pGameLayer = ApplicationLayer::GetInstance()->GetGameLayer();

    Collision colA = group.m_colA;
    Collision colB = group.m_colB;

    IView* pViewA = pGameLayer->FindView(colA.m_pActor->GetId());
    if (pViewA != nullptr)
    {
        pViewA->OnCollisionEnter(colB);
    }

    IView* pViewB = pGameLayer->FindView(colB.m_pActor->GetId());
    if (pViewB != nullptr)
    {
        pViewB->OnCollisionEnter(colA);
    }
}

void ContactListener::EndActorCollision(CollisionGroup& group)
{
    auto pGameLayer = ApplicationLayer::GetInstance()->GetGameLayer();

    Collision colA = group.m_colA;
    Collision colB = group.m_colB;

    IView* pViewA = pGameLayer->FindView(colA.m_pActor->GetId());
    if (pViewA)
    {
        pViewA->OnCollisionExit(colB);
    }

    IView* pViewB = pGameLayer->FindView(colB.m_pActor->GetId());
    if (pViewB)
    {
        pViewB->OnCollisionExit(colA);
    }
}

void ContactListener::BeginActorTrigger(CollisionGroup& group)
{
    auto pGameLayer = ApplicationLayer::GetInstance()->GetGameLayer();

    Collision colA = group.m_colA;
    Collision colB = group.m_colB;

    IView* pViewA = pGameLayer->FindView(colA.m_pActor->GetId());
    if (pViewA)
    {
        pViewA->OnTriggerEnter(colB);
    }

    IView* pViewB = pGameLayer->FindView(colB.m_pActor->GetId());
    if (pViewB)
    {
        pViewB->OnTriggerEnter(colA);
    }
}

void ContactListener::EndActorTrigger(CollisionGroup& group)
{
    auto pGameLayer = ApplicationLayer::GetInstance()->GetGameLayer();

    Collision colA = group.m_colA;
    Collision colB = group.m_colB;

    IView* pViewA = pGameLayer->FindView(colA.m_pActor->GetId());
    if (pViewA)
    {
        pViewA->OnTriggerExit(colB);
    }

    IView* pViewB = pGameLayer->FindView(colB.m_pActor->GetId());
    if (pViewB)
    {
        pViewB->OnTriggerExit(colA);
    }
}
