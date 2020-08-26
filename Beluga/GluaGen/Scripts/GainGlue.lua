--*******************************************************************************
-- Constants
--*******************************************************************************
kGainGlue = 'LUA_GAINGLUE';

--*******************************************************************************
-- GainGlue
--*******************************************************************************
GainGlue = {};
GainGlue._functionName = '';
GainGlue._memberType = '';
GainGlue._memberName = '';

function GainGlue:new(object)
    object = object or {}; -- Use table if passed in;
    setmetatable(object, self);
    self.__index = self; -- Invoke self for missing keys
    return object;
end

function GainGlue:WriteToFile(file, class)
    file:write('    static int ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(lua_touserdata(pState, 1));\n');
    file:write('        ', self._memberType, ' ', self._memberName, ' = static_cast<', self._memberType, '>(luaL_checknumber(pState, 2));\n');
    file:write('        pClass->', self._functionName, '(', self._memberName, ');\n');
    file:write('        lua_pop(pState, 2);\n');
    file:write('        return 0;\n');
    file:write('    }\n');   
end
