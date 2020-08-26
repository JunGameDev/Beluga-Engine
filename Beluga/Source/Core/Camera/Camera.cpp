#include "Core/Camera/Camera.h"
#include "Core/Layers/ApplicationLayer.h"
#include "Scripting/Scripting.h"

using namespace Bel;

namespace Lua
{
    static int CameraMove(lua_State* pState)
    {
        Camera2D& camera = Camera;

        float x = static_cast<float>(luaL_checknumber(pState, 1));
        float y = static_cast<float>(luaL_checknumber(pState, 2));
        camera.MovePos(x, y);
        lua_pop(pState, 2);

        return 0;
    }

    static int CameraSetPosition(lua_State* pState)
    {
        Camera2D& camera = Camera;

        float x = static_cast<float>(luaL_checknumber(pState, 1));
        float y = static_cast<float>(luaL_checknumber(pState, 2));
        camera.SetPosition(x, y);
        lua_pop(pState, 2);

        return 0;
    }

    static int CameraGetCenterX(lua_State* pState)
    {
        Camera2D& camera = Camera;

        lua_pushnumber(pState, camera.GetCenter().m_x);
        return 1;
    }

    static int CameraGetCenterY(lua_State* pState)
    {
        Camera2D& camera = Camera;

        lua_pushnumber(pState, camera.GetCenter().m_y);
        return 1;
    }

    static int CameraGetX(lua_State* pState)
    {
        Camera2D& camera = Camera;

        lua_pushnumber(pState, camera.GetRenderingPoint().m_x);
        return 1;
    }

    static int CameraGetY(lua_State* pState)
    {
        Camera2D& camera = Camera;

        lua_pushnumber(pState, camera.GetRenderingPoint().m_y);
        return 1;
    }
}

void Camera2D::RegisterWithLua()
{
    auto& scripting = ApplicationLayer::GetInstance()->GetGameLayer()->GetScriptingManager();
    scripting.GetGlobal("g_logic");
    
    scripting.CreateTable();
    scripting.AddToTable("this", this);
    scripting.AddToTable("Move", Lua::CameraMove);
    scripting.AddToTable("SetPosition", Lua::CameraSetPosition);
    scripting.AddToTable("GetCenterX", Lua::CameraGetCenterX);
    scripting.AddToTable("GetCenterY", Lua::CameraGetCenterY);
    scripting.AddToTable("GetX", Lua::CameraGetX);
    scripting.AddToTable("GetY", Lua::CameraGetY);
    scripting.AddToTable("Camera");

    scripting.PopAll();
}
