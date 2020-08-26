kGetGlue = 'LUA_GETGLUE';

--*******************************************************************************
-- GetGlue
--*******************************************************************************
GetGlue = {};
GetGlue._functionName = '';
GetGlue._memberType = '';
GetGlue._memberName = '';

function GetGlue:new(object)
    object = object or {}; -- use table if passed in
    setmetatable(object, self);
    self.__index = self; -- invoke self for missing keys
    return object;
end

function GetGlue:WriteToFile(file, class)
    file:write('    static ', self._memberType, ' ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(lua_touserdata(pState, 1));\n');
    file:write('        ', self._memberType, ' ', self._memberName, ' = pClass->', self._functionName, '();\n');
    file:write('        lua_pop(pState, 1);\n');
    file:write('        return ', self._memberName, ';\n');
    file:write('    }\n');
end
