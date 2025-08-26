cmake_minimum_required(VERSION 3.16)
project(BlackHoleSimulation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Platform-specific settings
if(WIN32)
    # Windows specific flags
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")
        # Use static runtime on Windows to avoid DLL dependencies
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    endif()
else()
    # Linux/Unix specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/Window.cpp
    src/shader.cpp
    src/BlackHole.cpp
    src/Star.cpp
)

# Header files (for IDE organization)
set(HEADERS
    include/Window.h
    include/shader.h
    include/BlackHole.h
    include/Star.h
)

# GLAD
add_library(glad STATIC
    libs/glad/src/glad.c
)
target_include_directories(glad PUBLIC libs/glad/include)

# Find packages
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLFW
if(WIN32)
    # On Windows, use the bundled GLFW
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(libs/glfw)
else()
    # On Linux, prefer system GLFW, fallback to bundled
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        message(STATUS "System GLFW not found, using bundled version")
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(libs/glfw)
    else()
        message(STATUS "Found system GLFW")
    endif()
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    libs/glm
    libs/glad/include
)

# Link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        glad
        glfw
        OpenGL::GL
    )
else()
    if(glfw3_FOUND)
        target_link_libraries(${PROJECT_NAME}
            glad
            glfw3
            OpenGL::GL
            ${CMAKE_DL_LIBS}
            pthread
        )
    else()
        target_link_libraries(${PROJECT_NAME}
            glad
            glfw
            OpenGL::GL
            ${CMAKE_DL_LIBS}
            pthread
        )
    endif()
endif()

# Copy shaders to output directory
file(GLOB SHADERS "res/shaders/*")
foreach(SHADER ${SHADERS})
    configure_file(${SHADER} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/ COPYONLY)
endforeach()

# Set working directory for debugging in Visual Studio
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Group files in IDEs
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

# Installation rules (optional)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY res/shaders DESTINATION bin)