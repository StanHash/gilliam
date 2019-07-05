cmake_minimum_required(VERSION 3.1)

project(gilliam)

option(USE_STATIC_LIBRARIES "When on, passes -static to the compiler" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(GILLIAM_SOURCES
	"main.cpp"
)

include_directories(${CMAKE_SOURCE_DIR})

if (USE_STATIC_LIBRARIES)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif ()

add_executable(${PROJECT_NAME} ${GILLIAM_SOURCES})
