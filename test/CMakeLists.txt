
file(GLOB TEST_SRC
    "./unit_tests/*test.cpp"
)

add_executable(utest gtest_main.cpp ${TEST_SRC})
    
target_include_directories(utest PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(utest PRIVATE gtest)
target_link_libraries(utest PRIVATE Threads::Threads)
target_link_libraries(utest PRIVATE k64lib)
set_target_properties(utest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


add_test(
  NAME
    utest
  COMMAND
    ${CMAKE_BINARY_DIR}/utest
  )