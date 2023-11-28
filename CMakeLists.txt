cmake_minimum_required(VERSION 3.23)
project(HelloVulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)


# =========== INCLUDE ============ #
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR}/../Third-Party/Include)

include_directories(include)
if (WIN32)
    include_directories(third_party/include)
elseif (APPLE)

endif()


add_subdirectory(${CMAKE_SOURCE_DIR}/src/shaders)


add_executable(HelloVulkan)
target_sources(HelloVulkan PRIVATE
        src/main.cpp
        src/m1k_window.cpp
        src/m1k_device.cpp
        src/m1k_pipeline.cpp
        src/m1k_application.cpp
        src/m1k_swap_chain.cpp
        src/m1k_model.cpp
        src/m1k_renderer.cpp
        src/m1k_camera.cpp
        src/simple_render_system.cpp
        src/keyboard_movement_controller.cpp
        )

if (WIN32)
    message(STATUS "==== Windows System ====")
    set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/third_party/lib/glfw3/glfw3.lib)
    target_link_libraries(HelloVulkan
            ${Vulkan_LIBRARIES}
            ${GLFW_LIB_PATH})
elseif (APPLE)
    message(STATUS "==== Mac System ====")
    find_package(glfw3 REQUIRED)
    target_link_libraries(HelloVulkan ${Vulkan_LIBRARIES} glfw)
else ()
    message(FATAL_ERROR "Unsupported Platform")
endif ()


add_dependencies(HelloVulkan Shaders)
