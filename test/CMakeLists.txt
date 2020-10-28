#${PROJECT_SOURCE_DIR}/src/chess_funcs.cpp
add_executable(testall gtest_main.cpp 
		 chess_funcs_test.cpp)
target_include_directories(testall PRIVATE ${PROJECT_SOURCE_DIR}/src)
#target_link_libraries(testall PRIVATE libgtest.a)
#target_link_libraries(testall PRIVATE libgtest_main.a)
target_link_libraries(testall PRIVATE gtest)
target_link_libraries(testall PRIVATE Threads::Threads)
target_link_libraries(testall PRIVATE k64lib)

add_test(
  NAME
    all_tests
  COMMAND
    ${CMAKE_CURRENT_BINARY_DIR}/testall
  )