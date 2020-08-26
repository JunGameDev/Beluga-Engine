set(PROJECT_NAME SandBox)

################################################################################
# Source groups
################################################################################
set(Include
    "Include/Game.h"
)
source_group("Include" FILES ${Include})

set(Src
    "Src/Game.cpp"
    "Src/Main.cpp"
)
source_group("Src" FILES ${Src})

set(ALL_FILES
    ${Include}
    ${Src}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE SandBox)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/Game/$ENV{PlatformShortName}_$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Game/$ENV{PlatformShortName}_$<CONFIG>/"
    OUTPUT_DIRECTORY_TEST    "${CMAKE_CURRENT_SOURCE_DIR}/Game/$ENV{PlatformShortName}_$<CONFIG>/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_TEST    "TRUE"
)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../Beluga/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        "_CONSOLE"
    ">"
    "$<$<CONFIG:Release>:"
        "BELUGA_TEST_BUILD;"
        ""
    ">"
    "$<$<CONFIG:Test>:"
        "BELUGA_TEST_BUILD;"
        ""
    ">"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        $<$<CONFIG:Test>:
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++17;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
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
        /DEBUG;
        /SUBSYSTEM:WINDOWS;
        /INCREMENTAL:NO
    )
endif()

################################################################################
# Post build events
################################################################################
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND   xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>..\\Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
    COMMAND   xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>..\\Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
    COMMAND   xcopy "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>..\\Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\*.dll" "$<SHELL_PATH:${OUTPUT_DIRECTORY}>" /y /i /d
)

################################################################################
# Custom build events
################################################################################
add_custom_command(
    OUTPUT "alwaysrun"
    COMMAND   "$<SHELL_PATH:${CMAKE_SOURCE_DIR}/>Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\GluaGen" "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source"
    COMMAND   "$<SHELL_PATH:${CMAKE_SOURCE_DIR}/>Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\GluaGen" "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source"
    COMMAND   "$<SHELL_PATH:${CMAKE_SOURCE_DIR}/>Beluga\\Libs\\${CMAKE_VS_PLATFORM_NAME}_$<CONFIG>\\GluaGen" "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Source"
)

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    GluaGen
    ResourcePacker
    BelugaTest
    Beluga
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Beluga
)