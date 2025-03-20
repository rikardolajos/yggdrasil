add_executable(hello_triangle)
target_sources(hello_triangle PRIVATE
    "hello_triangle.c"
)

add_custom_target(hello_triangle_shader ALL
    COMMENT "Compiling shaders"
    COMMAND slangc ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang -target spirv -o ${CMAKE_CURRENT_BINARY_DIR}/shader.spv
)
