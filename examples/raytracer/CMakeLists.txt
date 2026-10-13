add_executable(raytracer)
target_sources(raytracer PRIVATE
    "main.c"
)

foreach(file common.glsl hit.glsl raygen.rgen miss.rmiss shadow.rmiss closesthit.rchit anyhit.rahit texture.png)
    configure_file(${file} "${CMAKE_BINARY_DIR}/examples/raytracer/${file}" COPYONLY)
endforeach()
