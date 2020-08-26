--*******************************************************************************
-- Constants
--*******************************************************************************
kSetGlueParam_2 = 'LUA_SETGLUE_TWO';

--*******************************************************************************
-- GainGlue
--*******************************************************************************
SetGlueParam_2 = {};
SetGlueParam_2._functionName = '';
SetGlueParam_2._firstMemberType = '';
SetGlueParam_2._firstMemberName = '';
SetGlueParam_2._secondMemberType = '';
SetGlueParam_2._secondMemberName = '';

function SetGlueParam_2:new(object)
    object = object or {}; -- Use table if passed in;
    setmetatable(object, self);
    self.__index = self; -- Invoke self for missing keys
    return object;
end

function SetGlueParam_2:WriteToFile(file, class)
    file:write('    static int ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(\n');
    file:write('        lua_touserdata(pState, 1));\n');
    file:write('        ', self._firstMemberType, ' ', self._firstMemberName, ' = static_cast<', self._firstMemberType, '>(luaL_checknumber(pState, 2));\n');
    file:write('        ', self._secondMemberType, ' ', self._secondMemberName, ' = static_cast<', self._secondMemberType, '>(luaL_checknumber(pState, 3));\n');
    file:write('        pClass->', self._functionName, '(', self._firstMemberName, ', ', self._secondMemberName, ');\n');
    file:write('        lua_pop(pState, 3);\n');
    file:write('        return 0;\n');
    file:write('    }\n');   
end

