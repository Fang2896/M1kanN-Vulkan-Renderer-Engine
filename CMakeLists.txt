cmake_minimum_required(VERSION 3.23)
project(HelloVulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)


# =========== INCLUDE ============ #
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR}/../Third-Party/Include)

include_directories(${PROJECT_SOURCE_DIR}/include)
if (WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/include)
elseif (APPLE)

endif()


add_subdirectory(./shaders)


add_executable(HelloVulkan include/m1k_model.hpp)
target_sources(HelloVulkan PRIVATE
        main.cpp
        m1k_window.cpp
        m1k_device.cpp
        m1k_pipeline.cpp
        m1k_application.cpp
        m1k_swap_chain.cpp
        m1k_model.cpp
        m1k_renderer.cpp
        m1k_camera.cpp
        simple_render_system.cpp
        keyboard_movement_controller.cpp
        )

if (WIN32)
    message(STATUS "==== Windows System ====")
    set(GLFW_LIB_PATH ${PROJECT_SOURCE_DIR}/third_party/lib/glfw3/glfw3.lib)
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