cmake_minimum_required(VERSION 3.16)

project(Arx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_library(arx STATIC src/Arx.cpp src/Arx.h)
target_include_directories(arx PUBLIC src)

# test/ is not tracked yet, so a fresh clone builds the library only.
if(EXISTS "${PROJECT_SOURCE_DIR}/test/Main.cpp")
    add_executable(arx_test test/Main.cpp)
    target_link_libraries(arx_test PRIVATE arx)

    # The test opens out.a relative to its working directory.
    set(ARX_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/test/data")
    set_target_properties(arx_test PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${ARX_TEST_DATA_DIR}")

    enable_testing()
    add_test(NAME arx_test COMMAND arx_test WORKING_DIRECTORY "${ARX_TEST_DATA_DIR}")
endif()
