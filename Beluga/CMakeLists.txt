set(PROJECT_NAME Beluga)

################################################################################
# Source groups
################################################################################
set(Actors
    "Include/Actors/Actor.h"
    "Source/Actor/Actor.cpp"
)
source_group("Actors" FILES ${Actors})

set(Audio
    "Include/Audio/Audio.h"
    "Source/Audio/Audio.cpp"
)
source_group("Audio" FILES ${Audio})

set(Core__Camera
    "Include/Core/Camera/Camera.h"
    "Source/Core/Camera/Camera.cpp"
)
source_group("Core\\Camera" FILES ${Core__Camera})

set(Core__Layers
    "Include/Core/Layers/ApplicationLayer.h"
    "Include/Core/Layers/GameLayer.h"
    "Include/Core/Layers/View.h"
    "Source/Core/Layers/ApplicationLayer.cpp"
    "Source/Core/Layers/GameLayer.cpp"
)
source_group("Core\\Layers" FILES ${Core__Layers})

set(Core__Log
    "Include/Log/Logging.h"
    "Source/Log/Logging.cpp"
)
source_group("Core\\Log" FILES ${Core__Log})

set(Core__Math
    "Include/Core/Math/MathHelper.h"
    "Include/Core/Math/Vector2.h"
)
source_group("Core\\Math" FILES ${Core__Math})

set(Core__Utility
    "Include/Core/Util/GUID_Helper.h"
)
source_group("Core\\Utility" FILES ${Core__Utility})

set(Events
    "Include/Events/Events.h"
    "Include/Events/Processes.h"
    "Source/Events/Events.cpp"
    "Source/Events/Processes.cpp"
)
source_group("Events" FILES ${Events})

set(GUI
    "Include/GUI/GUI.h"
    "Source/GUI/GUI.cpp"
)
source_group("GUI" FILES ${GUI})

set(Graphics
    "Include/Graphics/Graphics.h"
    "Source/Graphics/Graphics.cpp"
)
source_group("Graphics" FILES ${Graphics})

set(Input
    "Include/Input/Input.h"
    "Source/Input/Input.cpp"
)
source_group("Input" FILES ${Input})

set(Level
    "Include/Levels/Level.h"
    "Source/Levels/Level.cpp"
)
source_group("Level" FILES ${Level})

set(Parshing
    "Include/Parshing/tinyxml2.h"
    "Source/Parshing/tinyxml2.cpp"
)
source_group("Parshing" FILES ${Parshing})

set(Physics
    "Include/Physics/Physics.h"
    "Include/Physics/PhysicsShape.h"
    "Source/Physics/Physics.cpp"
    "Source/Physics/PhysicsShape.cpp"
)
source_group("Physics" FILES ${Physics})

set(Resources
    "Include/Resources/Resource.h"
    "Source/Resources/Resource.cpp"
)
source_group("Resources" FILES ${Resources})

set(Scripting
    "Include/Scripting/Scripting.h"
    "Source/Scripting/Scripting.cpp"
)
source_group("Scripting" FILES ${Scripting})

set(Scripting__Lua
    "Include/Scripting/lauxlib.h"
    "Include/Scripting/lua.h"
    "Include/Scripting/lua.hpp"
    "Include/Scripting/luaconf.h"
    "Include/Scripting/lualib.h"
)
source_group("Scripting\\Lua" FILES ${Scripting__Lua})

set(System
    "Include/Systems/HardwareResource.h"
    "Include/Systems/System.h"
    "Source/Systems/HardwareResource.cpp"
    "Source/Systems/System.cpp"
)
source_group("System" FILES ${System})

set(ALL_FILES
    ${Actors}
    ${Audio}
    ${Core__Camera}
    ${Core__Layers}
    ${Core__Log}
    ${Core__Math}
    ${Core__Utility}
    ${Events}
    ${GUI}
    ${Graphics}
    ${Input}
    ${Level}
    ${Parshing}
    ${Physics}
    ${Resources}
    ${Scripting}
    ${Scripting__Lua}
    ${System}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Beluga)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/Libs/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Libs/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/"
    OUTPUT_DIRECTORY_TEST    "${CMAKE_CURRENT_SOURCE_DIR}/Libs/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_TEST    "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_image/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_mixer/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_ttf/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/zlib/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/Box2D/include"
)

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "$<$<CONFIG:Test>:"
            "BELUGA_TEST_BUILD;"
            "NDEBUG"
        ">"
        "_LIB;"
        "_MBCS"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86" OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "WIN32")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "$<$<CONFIG:Test>:"
            "BELUGA_TEST_BUILD;"
            "NDEBUG"
        ">"
        "x86;"
        "_LIB;"
        "_MBCS"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        $<$<CONFIG:Test>:
            /O2;
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++17;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Test>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()


################################################################################
# Dependencies
################################################################################
target_link_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2/lib/${CMAKE_VS_PLATFORM_NAME}/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_image/lib/${CMAKE_VS_PLATFORM_NAME}/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_mixer/lib/${CMAKE_VS_PLATFORM_NAME}/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/SDL2_ttf/lib/${CMAKE_VS_PLATFORM_NAME}/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/zlib/lib/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/Lua/lib/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Middleware/Box2D/lib/${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>/
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        SDL2
        SDL2_image
        SDL2_mixer
        SDL2_ttf
        zlibstat
        Lua
        box2d
)

################################################################################
# Post build events
################################################################################
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND    xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source\\Middleware\\SDL2\\lib\\${CMAKE_VS_PLATFORM_NAME}\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
    COMMAND    xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source\\Middleware\\SDL2_image\\lib\\${CMAKE_VS_PLATFORM_NAME}\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
    COMMAND    xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source\\Middleware\\SDL2_mixer\\lib\\${CMAKE_VS_PLATFORM_NAME}\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
    COMMAND    xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source\\Middleware\\Lua\\lib\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
)