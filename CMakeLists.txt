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

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
link_libraries(${Vulkan_LIBRARIES})

set(VULKAN_SDK_ROOT $ENV{VULKAN_SDK})
find_library(GLSLANG_LIB glslang HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(SPIRV_LIB SPIRV-Tools HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(SPIRV_OPT_LIB SPIRV-Tools-opt HINTS "${VULKAN_SDK_ROOT}/lib")
find_library(GLSLANG_DEFAULT_RESOURCE_LIMITS_LIB glslang-default-resource-limits HINTS "${VULKAN_SDK_ROOT}/lib")
link_libraries(${GLSLANG_LIB} ${SPIRV_LIB} ${SPIRV_OPT_LIB} ${GLSLANG_DEFAULT_RESOURCE_LIMITS_LIB})

add_subdirectory(submodules)
include_directories(submodules/glfw/include)
include_directories(submodules/stb)
include_directories(submodules/vecmat)
link_libraries(glfw)

include_directories(.)
add_subdirectory(example)
