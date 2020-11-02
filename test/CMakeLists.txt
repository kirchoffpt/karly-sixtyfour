
file(GLOB TEST_SRC
    "*test.cpp"
)

add_executable(testall gtest_main.cpp ${TEST_SRC})
    
target_include_directories(testall PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(testall PRIVATE gtest)
target_link_libraries(testall PRIVATE Threads::Threads)
target_link_libraries(testall PRIVATE k64lib)
set_target_properties(testall PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_test(
  NAME
    all_tests
  COMMAND
    ${CMAKE_BINARY_DIR}/testall
  )