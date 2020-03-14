cmake_minimum_required(VERSION 3.2)
project(GPIOPlusPlus)

set(CMAKE_CXX_STANDARD 17)

add_library(GPIOPlusPlus GPIO++.cpp GPIO++.hpp Utils.cpp Utils.hpp)

add_executable(GPIOPlusPlus_Test test.cpp)
target_link_libraries(GPIOPlusPlus_Test GPIOPlusPlus pthread)
