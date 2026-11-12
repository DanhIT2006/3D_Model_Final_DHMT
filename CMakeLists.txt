cmake_minimum_required(VERSION 3.20)
project(XeDieuKhien3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "main.cpp"
)

add_executable(XeDieuKhien3D ${SOURCES})

# Liên kết các thư viện OpenGL & FreeGLUT
find_package(OpenGL REQUIRED)
find_package(GLUT QUIET)

if(GLUT_FOUND)
    target_link_libraries(XeDieuKhien3D PRIVATE GLUT::GLUT OpenGL::GL OpenGL::GLU)
else()
    target_link_libraries(XeDieuKhien3D PRIVATE freeglut opengl32 glu32)
endif()
