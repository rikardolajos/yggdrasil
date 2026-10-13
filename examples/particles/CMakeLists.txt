add_executable(particles)
target_sources(particles PRIVATE
    "main.c"
)

foreach(file particles.comp particles.vert particles.frag)
    configure_file(${file} "${CMAKE_BINARY_DIR}/examples/particles/${file}" COPYONLY)
endforeach()
