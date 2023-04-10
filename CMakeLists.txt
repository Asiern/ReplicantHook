cmake_minimum_required(VERSION 3.12)
project(ReplicantHook VERSION 2.5 DESCRIPTION "NieR Replicant ver.1.22474487139 Memory Hook")

set(BUILD_TYPE "Release")
option(REPLICANTHOOK_STATIC_LIB "Build static library" ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(SRC
    src/ReplicantHook.cpp
)


# Create library
if(REPLICANTHOOK_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${SRC})
else()
    add_library(${PROJECT_NAME} SHARED ${SRC})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/ReplicantHook.hpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Configure Demo
if(BUILD_TYPE STREQUAL "Debug")
    set(DEMO_SOURCES src/Main.cpp)
    add_executable(${PROJECT_NAME}_DEMO ${DEMO_SOURCES})
    target_include_directories(${PROJECT_NAME}_DEMO PRIVATE include)
    target_link_libraries(${PROJECT_NAME}_DEMO ${PROJECT_NAME})
endif()