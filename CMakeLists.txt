cmake_minimum_required(VERSION 2.6)

project (ast_yet)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -O3")
set(CMAKE_LD_FLAGS "-")

# files
include_directories(
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_BINARY_DIR}"
)

#
file(GLOB SRC
	"*.cpp" "*/*.cpp"
	"*.hpp" "*/*.hpp"
)
add_executable(ast_yet ${SRC})

