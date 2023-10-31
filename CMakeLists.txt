cmake_minimum_required(VERSION 3.23)
project(HelloVulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR}/../Third-Party/Include)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/include)

add_executable(HelloVulkan)
target_sources(HelloVulkan PRIVATE  main.cpp)

target_link_libraries(HelloVulkan
        ${Vulkan_LIBRARY}
        ${PROJECT_SOURCE_DIR}/third_party/lib/glfw3/glfw3.lib
        )
