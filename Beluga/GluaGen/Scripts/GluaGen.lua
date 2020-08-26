require("GetGlue")
require("SetGlue")
require("TakeGlue")
require("GainGlue")
require("SetGlueParam_2")
require("FunctionGlue")

--*******************************************************************************
-- Constants
--*******************************************************************************
kRegister = 'LUA_REGISTER';
kClassName = 'class';

--*******************************************************************************
-- Class
--*******************************************************************************
Class = {};
Class._writeLuaBindings = false;
Class._name = '';
Class._registerFunctionBodyStart = {
    '{',
    '    auto& scripting = ApplicationLayer::GetInstance()->GetGameLayer()->GetScriptingManager();',
    '    scripting.GetGlobal("g_logic");',
    '    scripting.GetFromTable("actors");',
    '    scripting.GetFromTable(GetOwner()->GetId());\n',
    '    scripting.CreateTable();',
    '    scripting.AddToTable("this", this);'
};
Class._registerFunctionBodyEnd = {
    '    scripting.PopAll();',
    '}'
};

function Class:new(object)
    object = object or {}; -- use table if passed in

    object._getGlue = {}; -- create fresh tables here
    object._setGlue = {};
    object._gainGlue = {};
    object._takeGlue = {};
    object._setGlueParam_2 = {};
    object._lambdaGlue = {};
    object._functionGlue_0 = {};
    object._functionGlue_1 = {};

    setmetatable(object, self);
    self.__index = self; -- invoke self for missing keys
    return object;
end

-- Going through each of the words lookin for the next class keyword.
-- Returns the previous line index, When it's found.
function Class:Read(lines, startIndex)
    for lineIndex = startIndex + 1, #lines do
        local words = GetWords(lines[lineIndex]);
        for wordIndex = 1, #words do
            local word = words[wordIndex];
            if (word == kClassName) then 
                return lineIndex - 1; -- Found the next class, so bail
            elseif (string.find(word, kRegister)) then
                self._writeLuaBindings = true;
            end

            if(self._writeLuaBindings == true) then
                if (string.find(word, kGetGlue)) then
                    print("GetGlue found");
                    local getGlue = GetGlue:new({
                        _functionName = words[wordIndex + 1],
                        _memberType   = words[wordIndex + 2],
                        _memberName   = words[wordIndex + 3]
                    }); 
                    table.insert(self._getGlue, getGlue);
                elseif (string.find(word, kSetGlue)) then
                    print("SetGlue found");
                    local setGlue = SetGlue:new({
                        _functionName = words[wordIndex + 1],
                        _memberType = words[wordIndex + 2],
                        _memberName = words[wordIndex + 3]
                    });
                    table.insert(self._setGlue, setGlue);
                elseif (string.find(word, kGainGlue)) then
                    print("GainGlue found");
                    local gainGlue = GainGlue:new({
                        _functionName = words[wordIndex + 1],
                        _memberType = words[wordIndex + 2],
                        _memberName = words[wordIndex + 3]
                    });
                    table.insert(self._gainGlue, gainGlue);
                elseif (string.find(word, kTakeGlue)) then
                    print("TakeGlue found");
                    local takeGlue = TakeGlue:new({
                        _functionName = words[wordIndex + 1],
                        _memberType = words[wordIndex + 2],
                        _memberName = words[wordIndex + 3]
                    });
                    table.insert(self._takeGlue, takeGlue);
                elseif (string.find(word, kSetGlueParam_2)) then
                    print("SetGlue with two parameters found");
                    local setGlueParam_2 = SetGlueParam_2:new({
                        _functionName = words[wordIndex + 1],
                        _firstMemberType = words[wordIndex + 2],
                        _firstMemberName = words[wordIndex + 3],
                        _secondMemberType = words[wordIndex + 4],
                        _secondMemberName = words[wordIndex + 5],
                    });
                    table.insert(self._setGlueParam_2, setGlueParam_2);
                elseif (string.find(word, kLambdaGlue)) then
                    print("LambdaGlue found");
                    local lambdaGlue = LambdaGlue:new({
                        _functionName = words[wordIndex + 1],
                        _memberName = words[wordIndex + 2]
                    });
                    table.insert(self._lambdaGlue, lambdaGlue);
                elseif (string.find(word, kFunctionGlue_0)) then
                    print("FunctionGlue_0 found");
                    local functionGlue = FunctionGlue_0:new({
                        _functionName = words[wordIndex + 1]
                    });
                    table.insert(self._functionGlue_0, functionGlue);
                elseif (string.find(word, kFunctionGlue_1)) then
                    print("FunctionGlue_1 found");
                    local functionGlue = FunctionGlue_1:new({
                        _luaFunctionName = words[wordIndex + 1],
                        _functionName = words[wordIndex + 2],
                        _paramType = words[wordIndex + 3],
                        _paramName = words[wordIndex + 4]
                    });
                    table.insert(self._functionGlue_1, functionGlue);
                end
            end
        end
    end
    return #lines;
end

-- Going thorugh each of the workds looking for the next 'class' keyword
function GetWords(line)
    local words = {};
    -- Split by whitespace
    for word in line:gmatch("%S+") do
        if(not string.find(word, "//")) then
            for a in word:gmatch("[%w_]+") do
                table.insert(words, a);
            end
        end
    end
    return words;
end

--*******************************************************************************
-- HeaderFile
--*******************************************************************************
HeaderFile = {};
HeaderFile._filename = '';

function HeaderFile:new(object)
    object = object or {}; -- use table if passed in
    self._classes = {};
    setmetatable(object, self);
    self.__index = self; -- invoke self for missing keys
    return object;
end

-- Reading header file and finding indivisual classes.
function HeaderFile:ReadFile()
    local lines = {};

    for line in io.lines(self._filename) do
        table.insert(lines, line);
    end

    for lineIndex = 1, #lines do
        local words = GetWords(lines[lineIndex]);

        -- look for a Class
        for wordIndex, word in ipairs(words) do
            if(word == kClassName) then
                local class = Class:new({ _name = words[wordIndex + 1]} );
                lineIndex = class:Read(lines, lineIndex);
                table.insert(self._classes, class);
                break;
            end
        end
    end
end

function HeaderFile:WriteGenFile()
    for i, class in ipairs(self._classes) do
        if (class._writeLuaBindings) then
            -- % is an escape character so we can find the .
            local genFile = string.gsub(self._filename, '%.h', '.gen');
            local file = io.open(genFile, 'w');

            -- write our intro
            file:write('namespace Lua\n');
            file:write('{\n');
            ------------------------------------------------------------------
            -- Write our getters
            ------------------------------------------------------------------
            for i, getGlue in ipairs(class._getGlue) do
                getGlue:WriteToFile(file, class);
            end

            ------------------------------------------------------------------
            -- Write our setters
            ------------------------------------------------------------------
            for i, setGlue in ipairs(class._setGlue) do
                setGlue:WriteToFile(file, class);
            end

            ------------------------------------------------------------------
            -- Write take glues
            ------------------------------------------------------------------
            for i, takeGlue in ipairs(class._takeGlue) do
                takeGlue:WriteToFile(file, class);
            end

            ------------------------------------------------------------------
            -- Write gain glues
            ------------------------------------------------------------------
            for i, gainGlue in ipairs(class._gainGlue) do
                gainGlue:WriteToFile(file, class);
            end

            ------------------------------------------------------------------
            -- Write set glue with two parameters
            ------------------------------------------------------------------
            for i, setGlueParam_2 in ipairs(class._setGlueParam_2) do
                setGlueParam_2:WriteToFile(file, class);
            end

            ------------------------------------------------------------------
            -- Write lambda glue
            ------------------------------------------------------------------
            for i, lambdaGlue in ipairs(class._lambdaGlue) do
                lambdaGlue:WriteToFile(file, class);
            end
                    
            -- ------------------------------------------------------------------
            -- -- Write function glues
            -- ------------------------------------------------------------------
            for i, fileGlue in ipairs(class._functionGlue_0) do
                fileGlue:WriteToFile(file, class);
            end

            for i, fileGlue in ipairs(class._functionGlue_1) do
                fileGlue:WriteToFile(file, class);
            end

            -- write our outro
            file:write('}\n\n');
            
            ------------------------------------------------------------------
            -- write our class register functions
            ------------------------------------------------------------------
            file:write('void ', class._name, '::RegisterWithScript()\n');
            for i, line in ipairs(class._registerFunctionBodyStart) do
                file:write(line, '\n');
            end

            for i, getGlue in ipairs(class._getGlue) do
                file:write('    scripting.AddToTable("', getGlue._functionName,
                    '", Lua::', class._name, getGlue._functionName, ');\n');                
            end

            for i, setGlue in ipairs(class._setGlue) do
                file:write('    scripting.AddToTable("', setGlue._functionName,
                    '", Lua::', class._name, setGlue._functionName, ');\n');                
            end

            for i, takeGlue in ipairs(class._takeGlue) do
                file:write('    scripting.AddToTable("', takeGlue._functionName,
                    '", Lua::', class._name, takeGlue._functionName, ');\n');                
            end

            for i, gainGlue in ipairs(class._gainGlue) do
                file:write('    scripting.AddToTable("', gainGlue._functionName,
                    '", Lua::', class._name, gainGlue._functionName, ');\n');                
            end

            for i, setGlueParam_2 in ipairs(class._setGlueParam_2) do
                file:write('    scripting.AddToTable("', setGlueParam_2._functionName,
                    '", Lua::', class._name, setGlueParam_2._functionName, ');\n');                
            end

            for i, lambdaGlue in ipairs(class._lambdaGlue) do
                file:write('    scripting.AddToTable("', lambdaGlue._functionName,
                    '", Lua::', class._name, lambdaGlue._functionName, ');\n');                
            end

            for i, fileGlue in ipairs(class._functionGlue_0) do
                file:write('    scripting.AddToTable("', fileGlue._functionName,
                    '", Lua::', class._name, fileGlue._functionName, ');\n');                
            end

            for i, fileGlue in ipairs(class._functionGlue_1) do
                file:write('    scripting.AddToTable("', fileGlue._functionName,
                    '", Lua::', class._name, fileGlue._functionName, ');\n');                
            end

            file:write('    scripting.AddToTable("', class._name, '");\n');

            for i, line in ipairs(class._registerFunctionBodyEnd) do
                file:write(line, '\n');
            end
        end
    end
end

--*******************************************************************************
-- GluaGen
--*******************************************************************************

function Glue(path)
    HeaderFile:new();
    HeaderFile._filename = path;
    HeaderFile:ReadFile();
    HeaderFile:WriteGenFile();
end