set(PROJECT_NAME ResourcePacker)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "Main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE ResourcePacker)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/Beluga/Libs/$ENV{PlatformShortName}_$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Beluga/Libs/$ENV{PlatformShortName}_$<CONFIG>/"
    OUTPUT_DIRECTORY_TEST    "${CMAKE_SOURCE_DIR}/Beluga/Libs/$ENV{PlatformShortName}_$<CONFIG>/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_TEST    "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

################################################################################
# Compile definitions
################################################################################
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
    "_CONSOLE;"
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
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        $<$<CONFIG:Test>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /DEBUG;
        /SUBSYSTEM:CONSOLE
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Beluga
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Beluga
)