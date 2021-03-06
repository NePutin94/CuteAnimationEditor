cmake_minimum_required(VERSION 3.16)
project(CAE)

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "-O0 -g")
else()
    set(CMAKE_CXX_FLAGS "-O3 -mwindows -DNDEBUG -s")
endif()

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_SOURCE_DIR} ".include/*.h")
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "./src/*.cpp")
#file(GLOB_RECURSE IMGUI RELATIVE ${CMAKE_SOURCE_DIR} "./imgui/*.cpp")
file(GLOB_RECURSE IMGUI_SRC RELATIVE ${CMAKE_SOURCE_DIR} "./imgui-sfml-2.1/*.cpp")
file(GLOB_RECURSE IMGUI_HEADER RELATIVE ${CMAKE_SOURCE_DIR} "./imgui-sfml-2.1/*.h")

set(SFML_DIR D:\\Link\\SFML-2.5.1-GCC\\lib\\cmake\\SFML)
set(OpenCV_DIR D:\\Link\\opencv-4.4.0\\build)

find_package(OpenGL REQUIRED)
find_package(SFML 2.5 COMPONENTS system window graphics network audio)
find_package(OpenCV REQUIRED)
include_directories("./imgui-sfml-2.1")

include_directories("D:\\Link\\json-develop\\json-develop\\include")
include_directories(${OpenCV_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
MESSAGE(STATUS "Include dirs ${SOURCES}")
add_executable(CAE ${SOURCES} ${IMGUI_SRC})
target_link_libraries(CAE ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
target_link_libraries (CAE sfml-graphics sfml-window sfml-system)
target_link_libraries(CAE ${OpenCV_LIBS} )
