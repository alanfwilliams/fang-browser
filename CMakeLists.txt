cmake_minimum_required(VERSION 3.10)
project(FangBrowser C)

cmake_policy(SET CMP0148 NEW)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

include_directories(${Python3_INCLUDE_DIRS})

# Set the output directory for the module
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python_modules)

# Set the C standard since we're compiling C code
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    set(EXTRA_LIBS Ws2_32)
endif()

add_library(networking MODULE src/networking.c)

if(WIN32)
    set_target_properties(networking PROPERTIES
        SUFFIX ".pyd"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python_modules
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/python_modules
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/python_modules
    )
endif()

target_link_libraries(networking PRIVATE ${Python3_LIBRARIES} ${EXTRA_LIBS})

configure_file(${CMAKE_SOURCE_DIR}/python/test.py
               ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/test.py
               COPYONLY)

message(STATUS "Python module and test script will be output to: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")

