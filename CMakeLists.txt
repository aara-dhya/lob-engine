cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook VERSION 1.0)

# Enforce C++17 standard for modern features and performance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add the main executable
add_executable(lob_engine src/main.cpp)