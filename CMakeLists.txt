cmake_minimum_required(VERSION 3.14) 
project(LRParser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} 
    Parser.cpp 
    ParserBuilder.cpp 
    Lexer.cpp 
    LexerSources.cpp 
    LexerBuilder.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(LRApp main.cpp)
target_link_libraries(LRApp PRIVATE ${PROJECT_NAME})

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(test)