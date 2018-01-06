cmake_minimum_required(VERSION 2.8)

project(Variant)
add_executable(${PROJECT_NAME} main.cpp variant.h)

add_definitions(-DAlex_PKZDL=1)
add_definitions(-DTiliTim=1)
add_definitions(-DHOME=1)

#set (gtest_disable_pthreads on)
#add_subdirectory (googletest)
#config_compiler_and_linker()

set(CMAKE_C_COMPILER /usr/bin/gcc-7)
set(CMAKE_CXX_COMPILER /usr/bin/g++-7)

include_directories(/usr/include/c++/7/)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -pedantic -Wformat=2 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -Wfloat-equal -Wconversion -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif()

