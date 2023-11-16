# 设置glslc路径
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "/Users/m1kann/VulkanSDK/1.3.268.1/macOS/bin" "C:/VulkanSDK/1.3.268.0/Bin32")

if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found")
endif()

message(STATUS "Successfully Found GLSLC: Dir: ${GLSLC_EXECUTABLE}")

# 设置着色器源文件和编译目标文件
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sources")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/binaries")

message(STATUS "Shader Source Directory: ${SHADER_SOURCE_DIR}")
message(STATUS "Shader Binary Directory: ${SHADER_BINARY_DIR}")

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB SHADER_FILES
        "${SHADER_SOURCE_DIR}/*.vert"
        "${SHADER_SOURCE_DIR}/*.frag")

message(STATUS "Shader Files List: ${SHADER_FILES}")

foreach(SHADER IN LISTS SHADER_FILES)
    get_filename_component(SHADER_NAME ${SHADER} NAME)

    message(STATUS "Processing Shader: Name: ${SHADER_NAME}")

    set(BINARY_FILE "${SHADER_BINARY_DIR}/${SHADER_NAME}.spv")
    add_custom_command(
            OUTPUT ${BINARY_FILE}
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${BINARY_FILE}
            DEPENDS ${SHADER}
    )
    list(APPEND SPV_SHADERS ${BINARY_FILE})
endforeach()

add_custom_target(Shaders DEPENDS ${SPV_SHADERS})
