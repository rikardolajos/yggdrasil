add_executable(hello_triangle)
target_sources(hello_triangle PRIVATE
    "hello_triangle.c"
)

configure_file("shader.vert" "${CMAKE_BINARY_DIR}/examples/hello_triangle/shader.vert" COPYONLY)
configure_file("shader.frag" "${CMAKE_BINARY_DIR}/examples/hello_triangle/shader.frag" COPYONLY)
