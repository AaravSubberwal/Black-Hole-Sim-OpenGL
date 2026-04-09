cmake_minimum_required(VERSION 3.16)
project(BlackHoleSimulation LANGUAGES C CXX)

if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "BlackHoleSimulation is currently supported on Linux only.")
endif()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    src/AppPaths.cpp
    src/main.cpp
    src/Window.cpp
    src/shader.cpp
    src/BlackHole.cpp
    src/Camera.cpp
)

set(HEADERS
    include/AppPaths.h
    include/Window.h
    include/shader.h
    include/BlackHole.h
    include/Camera.h
)

set(SHADER_FILES
    res/computeShader.glsl
    res/vertexShader.glsl
    res/fragmentShader.glsl
)

add_library(glad STATIC libs/glad/src/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw/CMakeLists.txt")
    message(FATAL_ERROR "Missing vendored GLFW sources. Initialize the submodule under libs/glfw.")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    libs/glm
    libs/glad/include
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BLACK_HOLE_SIM_SOURCE_RES_DIR="${CMAKE_SOURCE_DIR}/res"
    BLACK_HOLE_SIM_INSTALL_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/blackholesim"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    pthread
)

foreach(SHADER_FILE IN LISTS SHADER_FILES)
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME)
    configure_file(
        "${CMAKE_SOURCE_DIR}/${SHADER_FILE}"
        "${CMAKE_BINARY_DIR}/res/${SHADER_NAME}"
        COPYONLY
    )
endforeach()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${SHADER_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/blackholesim)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
