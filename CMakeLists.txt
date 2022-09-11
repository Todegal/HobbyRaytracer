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
	"${CMAKE_CURRENT_BINARY_DIR}/dependencies/assimp/include"
)

# set variables that are needed for assimp
set(ZLIB_LIBRARIES zlibstatic)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)

add_subdirectory("dependencies/assimp")

# Include sub-projects.
add_subdirectory ("hobbyraytracer")

