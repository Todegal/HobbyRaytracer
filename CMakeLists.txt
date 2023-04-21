# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("hobbyraytracer")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	"dependencies/glm"
	"dependencies/stb"
	"dependencies/assimp/include"
	"dependencies/yaml-cpp/include"
	"${CMAKE_CURRENT_BINARY_DIR}/dependencies/assimp/include"
)

# set variables that are needed for dependencies
set(ZLIB_LIBRARIES zlibstatic CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

# add Assimp as a subdirectory
add_subdirectory("dependencies/assimp")

# add Yaml-cpp as a subdirectory
add_subdirectory("dependencies/yaml-cpp")

# Include sub-projects.
add_subdirectory ("hobbyraytracer")

