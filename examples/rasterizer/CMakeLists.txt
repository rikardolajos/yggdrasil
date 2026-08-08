add_executable(rasterizer)
target_sources(rasterizer PRIVATE
    "main.c"
)

configure_file("shader.vert" "${CMAKE_BINARY_DIR}/examples/rasterizer/shader.vert" COPYONLY)
configure_file("shader.frag" "${CMAKE_BINARY_DIR}/examples/rasterizer/shader.frag" COPYONLY)
configure_file("texture.png" "${CMAKE_BINARY_DIR}/examples/rasterizer/texture.png" COPYONLY)
