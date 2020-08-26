//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileName: Scripting.cpp
// Create On: 4/7/2020 12:41 PM
// Last modified: 
// Description: 
//      Implementation of ScriptingManager class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <iostream>
#include "Scripting/Scripting.h"
#include "Core/Layers/ApplicationLayer.h"

using namespace Bel;

ScriptingManager::ScriptingManager()
    : m_pState(nullptr, nullptr)
    , m_stackSize(0)
{
}

//--------------------------------------------------------------------------------------------
// Creates state and add the standard libraries.
//--------------------------------------------------------------------------------------------
bool ScriptingManager::Initialize()
{
    m_pState = std::unique_ptr<lua_State, decltype(&lua_close)>(luaL_newstate(), &lua_close);
    luaL_openlibs(m_pState.get());
    return true;
}

//--------------------------------------------------------------------------------------------
// Load buffer from a compressed script
//--------------------------------------------------------------------------------------------
bool ScriptingManager::RunScript(std::shared_ptr<ResourceHandle> pResHandle)
{
    bool error = luaL_loadbuffer(m_pState.get(), pResHandle->GetData().data(), pResHandle->GetData().size(), "script")
                 || lua_pcall(m_pState.get(), 0, 0, 0);

    if (error)
    {
        PrintLuaError();
    }

    return !error;
}

//--------------------------------------------------------------------------------------------
// Prints error message from Lua.
//--------------------------------------------------------------------------------------------
void ScriptingManager::PrintLuaError()
{
    const char* pError = lua_tostring(m_pState.get(), -1);
    auto application = ApplicationLayer::GetInstance();
    if (application != nullptr)
    {
        LOG(pError);
    }
    else
    {
        std::cout << pError << std::endl;
    }

    lua_pop(m_pState.get(), 1);
}

//--------------------------------------------------------------------------------------------
// Create an empty table on the stack, then adjust our stack size.
//--------------------------------------------------------------------------------------------
void ScriptingManager::CreateTable()
{
    lua_newtable(m_pState.get());
    ++m_stackSize;
}

//--------------------------------------------------------------------------------------------
//      Followings are a subset of all the possible things can be added to a table. 
//--------------------------------------------------------------------------------------------
#pragma region AdderGetter
//--------------------------------------------------------------------------------------------
// Add whatever on the stack. This will be used when addinga table to another table.
//--------------------------------------------------------------------------------------------
void ScriptingManager::AddToTable(const char* pKey)
{
    // This will add whatever is currently on the stack.
    // useful for adding tables to tables.
    lua_setfield(m_pState.get(), -2, pKey);
    --m_stackSize;
}

//--------------------------------------------------------------------------------------------
// Add a C function
//--------------------------------------------------------------------------------------------
void ScriptingManager::AddToTable(const char* pKey, lua_CFunction pFunction)
{
    lua_pushcfunction(m_pState.get(), pFunction);
    lua_setfield(m_pState.get(), -2, pKey);
}

//--------------------------------------------------------------------------------------------
// Add light user data
//--------------------------------------------------------------------------------------------
void ScriptingManager::AddToTable(const char* pKey, void* pUserData)
{
    lua_pushlightuserdata(m_pState.get(), pUserData);
    lua_setfield(m_pState.get(), -2, pKey);
}

//--------------------------------------------------------------------------------------------
// Retrieve the value by key of the table at the top of stack.
//--------------------------------------------------------------------------------------------
void ScriptingManager::GetFromTable(const char* pKey)
{
    lua_getfield(m_pState.get(), -1, pKey);
    ++m_stackSize;
}

//--------------------------------------------------------------------------------------------
// Retrieve the value by index of the table at the top of stack.
//--------------------------------------------------------------------------------------------
void ScriptingManager::GetFromTable(uint32_t index)
{
    lua_geti(m_pState.get(), -1, index);
    ++m_stackSize;
}
#pragma endregion

void ScriptingManager::GetGlobal(const char* pName)
{
    lua_getglobal(m_pState.get(), pName);
    ++m_stackSize;
}

void ScriptingManager::SetGlobal(const char* pName)
{
    lua_setglobal(m_pState.get(), pName);
    --m_stackSize;
}

void ScriptingManager::PushNumber(double number)
{
    lua_pushnumber(m_pState.get(), number);
    ++m_stackSize;
}

void ScriptingManager::PushString(const char* pStr)
{
    lua_pushstring(m_pState.get(), pStr);
    ++m_stackSize;
}

void ScriptingManager::PushBoolean(bool value)
{
    lua_pushboolean(m_pState.get(), value);
    ++m_stackSize;
}

void ScriptingManager::PushLightUserData(void* pData)
{
    lua_pushlightuserdata(m_pState.get(), pData);
    ++m_stackSize;
}

//--------------------------------------------------------------------------------------------
// Push the function to the stack.
//--------------------------------------------------------------------------------------------
void ScriptingManager::StartFunction(const char* pFunction)
{
    GetGlobal(pFunction);
}

bool ScriptingManager::CallFunction(int32_t numReturn)
{
    if (lua_pcall(m_pState.get(), m_stackSize - 1, numReturn, 0) != 0)
    {
        PrintLuaError();
        m_stackSize = 0;
        return false;
    }

    m_stackSize = numReturn;
    return true;
}

void ScriptingManager::TraceStack()
{
    /*
        ---- Begin Stack ----
        Stack size: 3
        
        +---------------------------------------+
        |  Index  |  I-Index  |   Value         |
        +---------------------------------------+
        |    3    |    -1     | Master Sword    |
        |    2    |    -2     | table           |
        |    1    |    -3     | function        |
        +---------------------------------------+
        ---- End Stack ----
    */
    int top = lua_gettop(m_pState.get());
    LOG("---- Begin Stack ----");
    LOG("Stack size: " + std::to_string(top));

    LOG("+---------------------------------------+");
    LOG("|  Index  |  I-Index  |\t  Value  \t|");
    LOG("+---------------------------------------+");

    for (int i = top; i >= 1; i--)
    {
        int type = lua_type(m_pState.get(), i);
        switch (type)
        {
        case LUA_TSTRING:
            LOG("|    " + std::to_string(i) + "    |" + "    " + std::to_string(i - (top + 1)) + "     |\t" + lua_tostring(m_pState.get(), i) + "\t|");
            break;

        case LUA_TBOOLEAN:
            LOG("|    " + std::to_string(i) + "    |" + "    " + std::to_string(i - (top + 1)) + "     |\t" + (lua_toboolean(m_pState.get(), i) ? "true" : "false") + "\t|");
            break;

        case LUA_TNUMBER:
            LOG("|    " + std::to_string(i) + "    |" + "    " + std::to_string(i - (top + 1)) + "     |\t" + std::to_string(lua_tonumber(m_pState.get(), i)) + "\t\t|");
            break;

        case LUA_TFUNCTION:
            LOG("|    " + std::to_string(i) + "    |" + "    " + std::to_string(i - (top + 1)) + "     |\t" + (lua_typename(m_pState.get(), type)) + "\t|");
            break;
        
        default:
            LOG("|    " + std::to_string(i) + "    |" + "    " + std::to_string(i - (top + 1)) + "     |\t" + (lua_typename(m_pState.get(), type)) + "\t\t|");
            break;
        }
    }
    
    LOG("+---------------------------------------+");
    LOG("---- End Stack ----");
    LOG("========================================================");
}

void ScriptingManager::PrintElementsInTable()
{
    lua_pushnil(m_pState.get());
    
    while (lua_next(m_pState.get(), -2) != 0)
    {
        if (lua_isstring(m_pState.get(), -1))
            std::cout << lua_tostring(m_pState.get(), -2) << " = " << lua_tostring(m_pState.get(), -1) << std::endl;
        else if (lua_isnumber(m_pState.get(), -1))
            std::cout << lua_tostring(m_pState.get(), -2) << " = " << lua_tonumber(m_pState.get(), -1) << std::endl;
        else if (lua_istable(m_pState.get(), -1))
            PrintElementsInTable();
    
        lua_pop(m_pState.get(), 1);
    }
}

void ScriptingManager::PopAll()
{
    lua_pop(m_pState.get(), m_stackSize);
    assert(lua_gettop(m_pState.get()) == 0);
    m_stackSize = 0;
}