add_executable(example)
target_sources(example PRIVATE
    "main.c"
)

configure_file("shader.vert" "${CMAKE_BINARY_DIR}/example/shader.vert" COPYONLY)
configure_file("shader.frag" "${CMAKE_BINARY_DIR}/example/shader.frag" COPYONLY)
configure_file("texture.png" "${CMAKE_BINARY_DIR}/example/texture.png" COPYONLY)
