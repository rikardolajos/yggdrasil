cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if (MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

project("Yggdrasil" VERSION 0.0.1)

option(GFX_BAKE_PIPELINE_STATE "Bake every pipeline state that can be baked, to test that path on any device" OFF)
option(GFX_EXIT_ON_ERROR "Exit on errors instead of breaking into the debugger, for automated runs" OFF)
if (GFX_BAKE_PIPELINE_STATE)
    add_compile_definitions(GFX_BAKE_PIPELINE_STATE)
endif()
if (GFX_EXIT_ON_ERROR)
    add_compile_definitions(GFX_EXIT_ON_ERROR)
endif()

# Added before the libraries below, which GLFW does not need and would otherwise
# pass on to the examples a second time
add_subdirectory(submodules)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
link_libraries(${Vulkan_LIBRARIES})

set(VULKAN_SDK_ROOT $ENV{VULKAN_SDK})
find_library(GLSLANG_LIB glslang REQUIRED HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(GLSLANG_DEFAULT_RESOURCE_LIMITS_LIB glslang-default-resource-limits REQUIRED HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(SPIRV_OPT_LIB SPIRV-Tools-opt REQUIRED HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(SPIRV_LIB SPIRV-Tools REQUIRED HINTS "${VULKAN_SDK_ROOT}/lib")

# Static libraries are searched once, in order, so each one comes before the
# ones it depends on
link_libraries(${GLSLANG_LIB} ${GLSLANG_DEFAULT_RESOURCE_LIMITS_LIB} ${SPIRV_OPT_LIB} ${SPIRV_LIB})

# glslang and SPIRV-Tools are C++ libraries, so a C program linking them needs
# the C++ runtime, which MSVC links on its own
if (APPLE)
    link_libraries(c++)
elseif (UNIX)
    find_package(Threads REQUIRED)
    link_libraries(stdc++ m Threads::Threads)
endif()

include_directories(submodules/glfw/include)
include_directories(submodules/stb)
include_directories(submodules/vecmat)
link_libraries(glfw)

include_directories(.)
add_subdirectory(examples/rasterizer)
add_subdirectory(examples/raytracer)
add_subdirectory(examples/particles)
add_subdirectory(examples/shadows)
