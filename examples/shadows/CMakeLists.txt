add_executable(shadows)
target_sources(shadows PRIVATE
    "main.c"
)

foreach(file common.glsl shadow.vert scene.vert scene.frag texture.png)
    configure_file(${file} "${CMAKE_BINARY_DIR}/examples/shadows/${file}" COPYONLY)
endforeach()
