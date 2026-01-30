set(TEST_PROJECT_NAME "LRTest")

add_executable(${TEST_PROJECT_NAME} ParserTest.cpp LexerTest.cpp)

target_link_libraries(${TEST_PROJECT_NAME} 
    PRIVATE 
        ${PROJECT_NAME} 
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})