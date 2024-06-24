cmake_minimum_required(VERSION 3.21)
project(encoding C CXX)

# Руководство по CMake для разработчиков C++ библиотек - https://habr.com/ru/articles/683204/
# https://github.com/pananton/cpp-lib-template/blob/main/CMakeLists.txt

# Сохранение древовидной структуры в Visual Studio с CMake или создание папок в проекте - https://habr.com/ru/articles/754272/


# string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)
# use PROJECT_IS_TOP_LEVEL instead (requires CMake v3.21)


if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


### End of common part


set(MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB_RECURSE sources "${MODULE_ROOT}/*.c*")
list(FILTER sources EXCLUDE REGEX ".*_unittest\\.cc$")
list(FILTER sources EXCLUDE REGEX ".*compact_enc_det_fuzz_test\\.cc$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "${PROJECT_NAME}/Sources" FILES ${sources})

file(GLOB_RECURSE headers "${MODULE_ROOT}/*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "${PROJECT_NAME}/Headers" FILES ${headers})


add_library(${PROJECT_NAME} ${sources} ${headers})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${MODULE_ROOT})

