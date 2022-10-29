cmake_minimum_required(VERSION 3.14)

project(BooksManagmentSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(_target "BooksManagmentSystem")

find_package(Qt5 COMPONENTS Core Sql REQUIRED)

configure_file(cmakevars.h.in cmakevars.h)

add_executable(${_target}
  main.cpp cmakevars.h
)

target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${_target} Qt::Core Qt::Sql)
