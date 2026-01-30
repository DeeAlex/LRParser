cmake_minimum_required(VERSION 4.1)
project(parser VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(parser Parser.cpp ParserBuilder.cpp Lexer.cpp LexerSources.cpp LexerBuilder.cpp main.cpp)
