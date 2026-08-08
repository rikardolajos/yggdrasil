add_executable(raytracer)
target_sources(raytracer PRIVATE
    "main.c"
)

configure_file("shader.vert" "${CMAKE_BINARY_DIR}/examples/raytracer/shader.vert" COPYONLY)
configure_file("shader.frag" "${CMAKE_BINARY_DIR}/examples/raytracer/shader.frag" COPYONLY)
configure_file("texture.png" "${CMAKE_BINARY_DIR}/examples/raytracer/texture.png" COPYONLY)
