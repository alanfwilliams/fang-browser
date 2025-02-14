CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

# Set the project name
project(FangBrowser C)

# include the python library
cmake_policy(SET CMP0148 NEW)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# add to include the python headers
include_directories(${Python3_INCLUDE_DIRS})

# add to link the python library
link_libraries(${Python3_LIBRARIES})

# Build the c file in docs/learning
add_subdirectory(docs/learning)

