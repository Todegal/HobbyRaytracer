# CMakeList.txt : CMake project for hobbyraytracer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(SOURCES
	"main.cpp" 
	"bvh.cpp"
	"hittableList.cpp" 
	"material.cpp" 
	"sphere.cpp" 
	"texture.cpp" 
	"constantMedium.cpp"
	"translate.cpp"
	"rotateY.cpp")

set(HEADERS
	"aabb.h"
	"aarect.h"
	"box.h"
	"bvh.h"
	"camera.h"
	"constantMedium.h"
	"hittable.h"
	"hittableList.h"
	"material.h"
	"ray.h"
	"rotateY.h"
	"sphere.h"
	"vendor/stb_image.h"
	"vendor/stb_image_write.h"
	"hobbyraytracer.h"
	"texture.h"
	"translate.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../glm)
	
add_executable (hobbyraytracer ${SOURCES} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hobbyraytracer PROPERTY CXX_STANDARD 20)
endif()