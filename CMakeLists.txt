cmake_minimum_required(VERSION 3.26)
project(MultiTypeOrderbook)

set(CMAKE_CXX_STANDARD 23)

add_executable(MultiTypeOrderbook src/main.cpp
        src/include/order.hpp
        src/include/portfolio.hpp
)
