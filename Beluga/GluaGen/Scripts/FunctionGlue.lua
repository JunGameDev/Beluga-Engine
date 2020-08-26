--*******************************************************************************
-- Constants
--*******************************************************************************
kLambdaGlue = 'LUA_LAMBDAGLUE';
kFunctionGlue_0 = 'LUA_FUNCTIONGLUE_0';
kFunctionGlue_1 = 'LUA_FUNCTIONGLUE_1';

--*******************************************************************************
-- LambdaGlue
--*******************************************************************************
LambdaGlue = {};
LambdaGlue._functionName = '';
LambdaGlue._memberName = '';

function LambdaGlue:new(object)
    object = object or {}; -- Use table if passed in;
    setmetatable(object, self);
    self.__index = self; -- Invoke self for missing keys
    return object;
end

function LambdaGlue:WriteToFile(file, class)
    file:write('    static int ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(lua_touserdata(pState, 1));\n');
    file:write('        pClass->m_', self._memberName, '();\n');
    file:write('        lua_pop(pState, 1);\n');
    file:write('        return 0;\n');
    file:write('    }\n');
end

--*******************************************************************************
-- FunctionGlue with no parameter
--*******************************************************************************
FunctionGlue_0 = {};
FunctionGlue_0._functionName = '';

function FunctionGlue_0:new(object)
    object = object or {}; -- Use table if passed in;
    setmetatable(object, self);
    self.__index = self; -- Invoke self for missing keys
    return object;
end

function FunctionGlue_0:WriteToFile(file, class)
    file:write('    static int ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(lua_touserdata(pState, 1));\n');
    file:write('        pClass->', self._functionName, '();\n');
    file:write('        lua_pop(pState, 1);\n');
    file:write('        return 0;\n');
    file:write('    }\n');
end

--*******************************************************************************
-- FunctionGlue with one parameter
--*******************************************************************************
FunctionGlue_1 = {};
FunctionGlue_1._luaFunctionName = '';
FunctionGlue_1._functionName = '';
FunctionGlue_1._paramType = '';
FunctionGlue_1._paramName = '';


function FunctionGlue_1:new(object)
    object = object or {}; -- Use table if passed in;
    setmetatable(object, self);
    self.__index = self; -- Invoke self for missing keys
    return object;
end

function FunctionGlue_1:WriteToFile(file, class)
    file:write('    static int ', class._name, self._functionName, '(lua_State* pState)\n');
    file:write('    {\n');
    file:write('        auto pClass = reinterpret_cast<', class._name, '*>(lua_touserdata(pState, 1));\n');
    print("------------------------------");
    print(self._functionName);
    print(self._paramType);
    print(self._paramName);
    print("------------------------------");

    if(self._paramType == 'int') then
        file:write('        ', self._paramType, ' ', self._paramName, ' = static_cast<', self._paramType, '>(luaL_checknumber(pState, 2));\n');
    elseif(self._paramType == 'char') then
        file:write('        const ', self._paramType, '* ', self._paramName, ' = static_cast<const ', self._paramType, '*','>(lua_tostring(pState, 2));\n');
    end
    file:write('        pClass->', self._functionName, '(', self._paramName, ');\n');
    file:write('        lua_pop(pState, 2);\n');
    file:write('        return 0;\n');
    file:write('    }\n');   
end
