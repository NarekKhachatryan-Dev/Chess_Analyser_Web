cmake_minimum_required(VERSION 3.16...3.31)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(ChessLab LANGUAGES CXX)

include(FetchContent)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

add_subdirectory(engine)
add_subdirectory(cli)
add_subdirectory(web)
add_subdirectory(tests)
