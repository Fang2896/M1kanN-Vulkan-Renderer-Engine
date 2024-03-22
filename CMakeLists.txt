cmake_minimum_required(VERSION 3.23)
project(M1kanN_Vulkan_Engine)

set(CMAKE_CXX_STANDARD 17)

set(MY_ENGINE_NAME M1kanN_Vulkan_Engine)
add_executable(${MY_ENGINE_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)


# =========== INCLUDE ============ #
find_package(Vulkan REQUIRED)
include_directories(
        ${Vulkan_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIR}/../Third-Party/Include

        src/
        src/core
        src/objects
        src/systems
        src/ui
        src/utils

        third_party/include
        third_party/include/stb
        third_party/include/model_loader
        third_party/include/imgui
        third_party/include/ImGuiFileDialog
        third_party/include/dirent
        )

if (WIN32)
    include_directories(third_party/include/GLFW)
elseif (APPLE)

endif()


add_subdirectory(${CMAKE_SOURCE_DIR}/shaders)

set(COMMON_SOURCES
        # imgui
        third_party/include/imgui/imgui.cpp
        third_party/include/imgui/imgui_demo.cpp
        third_party/include/imgui/imgui_draw.cpp
        third_party/include/imgui/imgui_widgets.cpp
        third_party/include/imgui/imgui_tables.cpp
        third_party/include/imgui/imgui_impl_glfw.cpp
        third_party/include/imgui/imgui_impl_vulkan.cpp
        third_party/include/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_sources(${MY_ENGINE_NAME} PRIVATE ${COMMON_SOURCES}
        src/ui/m1k_window.cpp
        src/ui/keyboard_movement_controller.cpp

        src/core/m1k_device.cpp
        src/core/m1k_pipeline.cpp
        src/core/m1k_renderer.cpp
        src/core/m1k_swap_chain.cpp
        src/core/m1k_buffer.cpp
        src/core/m1k_descriptor.cpp

        src/main.cpp
        src/m1k_application.cpp
        src/objects/m1k_model.cpp
        src/objects/m1k_texture.cpp
        src/objects/m1k_mesh.cpp
        src/ui/m1k_camera.cpp
        src/objects/m1k_game_object.cpp
        src/utils/m1k_utils.cpp

        src/systems/point_light_system.cpp
        # src/systems/pbr_render_system.cpp
        src/systems/bindless_pbr_render_system.cpp
)


if (WIN32)
    message(STATUS "==== Windows System ====")

    set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/third_party/lib/glfw3/glfw3.lib)

    target_link_libraries(${MY_ENGINE_NAME}
            ${Vulkan_LIBRARIES}
            ${GLFW_LIB_PATH})

elseif (APPLE)
    message(STATUS "==== Mac System ====")
    find_package(glfw3 REQUIRED)
    target_link_libraries(${MY_ENGINE_NAME} ${Vulkan_LIBRARIES} glfw)
else ()
    message(FATAL_ERROR "Unsupported Platform")
endif ()


add_dependencies(${MY_ENGINE_NAME} Shaders)
