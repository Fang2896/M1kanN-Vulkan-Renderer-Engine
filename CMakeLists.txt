cmake_minimum_required(VERSION 3.23)
project(HelloVulkan)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_executable(HelloVulkan)
# =========== INCLUDE ============ #
target_include_directories(HelloVulkan PRIVATE
        ${Vulkan_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIR}/../Third-Party/Include
        src/include
        third_party/include/obj_loader
        third_party/include/stb
        third_party/include/imgui
        )

if (WIN32)
    include_directories(third_party/include/GLFW)
elseif (APPLE)

endif()


add_subdirectory(${CMAKE_SOURCE_DIR}/shaders)

target_sources(HelloVulkan PRIVATE
        # imgui
        third_party/include/imgui/imgui.cpp
        third_party/include/imgui/imgui_demo.cpp
        third_party/include/imgui/imgui_draw.cpp
        third_party/include/imgui/imgui_widgets.cpp
        third_party/include/imgui/imgui_tables.cpp
        third_party/include/imgui/imgui_impl_glfw.cpp
        third_party/include/imgui/imgui_impl_vulkan.cpp

        src/main.cpp
        src/m1k_window.cpp
        src/m1k_device.cpp
        src/m1k_pipeline.cpp
        src/m1k_application.cpp
        src/m1k_swap_chain.cpp
        src/m1k_model.cpp
        src/m1k_renderer.cpp
        src/m1k_camera.cpp
        src/m1k_game_object.cpp
        src/m1k_buffer.cpp
        src/m1k_descriptor.cpp
        src/systems/point_light_system.cpp
        src/systems/simple_render_system.cpp
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
