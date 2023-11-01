cmake_minimum_required(VERSION 3.23)
project(HelloVulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR}/../Third-Party/Include)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(HelloVulkan)
target_sources(HelloVulkan PRIVATE  main.cpp)

if(WIN32)
    set(GLFW_LIB_PATH ${PROJECT_SOURCE_DIR}/third_party/lib/glfw3/glfw3.lib)
    target_link_libraries(HelloVulkan ${Vulkan_LIBRARIES} ${GLFW_LIB_PATH})
elseif(APPLE)
    find_package(glfw3 REQUIRED)
    target_link_libraries(HelloVulkan ${Vulkan_LIBRARIES} glfw)
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()
