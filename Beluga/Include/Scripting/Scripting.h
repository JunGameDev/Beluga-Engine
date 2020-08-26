//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileName: Scripting.h
// Last modified: 4/19/2020 3:57 AM
// Description: 
//      Manager that helps binding between C++ and Lua. This is responsible not only initializing and shutting
//      down lua stack, but also calling lua function from C++ side and C++ function from lua side.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include "Scripting/lua.hpp"
#include "Resources/Resource.h"

#define LUA_REGISTER() \
    virtual void RegisterWithScript() override;

#define LUA_GETGLUE(functionName, memberType, memberName) \
    memberType functionName() const { return m_##memberName;}

#define LUA_SETGLUE(functionName, memberType, memberName) \
    void functionName(const memberType& memberName) { m_##memberName = memberName; }

#define LUA_GAINGLUE(functionName, memberType, memberName) \
    void functionName(const memberType& memberName) { m_##memberName += memberName; }

#define LUA_TAKEGLUE(functionName, memberType, memberName) \
    void functionName(const memberType& memberName) { m_##memberName -= memberName; }

#define LUA_SETGLUE_TWO(functionName, memberType1, memberName1, memberType2, memberName2) \
    void functionName(memberType1 memberName1, memberType2 memberName2) { m_##memberName1 = memberName1; m_##memberName2 = memberName2; }

#define LUA_LAMBDAGLUE(functionName, memberName) \
    void functionName() { m_##memberName(); }

#define LUA_FUNCTIONGLUE_0(luaFuncName, functionName) \
    void luaFuncName() { this->functionName(); }

#define LUA_FUNCTIONGLUE_1(luaFuncName, functionName, paramType, param) \
    void luaFuncName(paramType param) { this->functionName(param); }

namespace Bel
{
    //-----------------------------------------------------------------------------------------
    // ScriptingManager definition
    // 
    // [ Description ] 
    // - Keeping track of the Lua state and the size of the stack. 
    // - Add any element into the table on the stack.
    // - Get an element from the talbe on the stack.
    // - Start and call the user-defined function.
    //
    // TODO: 
    // [] Consider adding functions that runs scripts directly from a file or from
    //    string in memory. These can be useful for iteration or advanced features
    //    like detecting changes to the Lua file and how swapping it during runtime.
    //-----------------------------------------------------------------------------------------
    class ScriptingManager
    {
    private:
        std::unique_ptr<lua_State, decltype(&lua_close)> m_pState;
        int32_t m_stackSize;

    public:
        ScriptingManager();
        bool Initialize();

        // Helper fucntions
        bool RunScript(std::shared_ptr<ResourceHandle> pResHandle);
        void PrintLuaError();
        void CreateTable();

        void AddToTable(const char* pKey);
        void AddToTable(const char* pKey, lua_CFunction pFunction);
        void AddToTable(const char* pKey, void* pUserData);

        void GetFromTable(const char* pKey);
        void GetFromTable(uint32_t index);

        void GetGlobal(const char* pName);
        void SetGlobal(const char* pName);

        void PushNumber(double number);
        void PushString(const char* pStr);
        void PushBoolean(bool value);
        void PushLightUserData(void* pData);

        void StartFunction(const char* pFunction);
        bool CallFunction(int32_t numReturn);

        void TraceStack();
        void PrintElementsInTable();

        void PopAll();
    };
}

