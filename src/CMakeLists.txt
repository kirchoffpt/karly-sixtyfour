
add_library(k64lib "")
target_sources(
	k64lib
	PRIVATE 
    search.cpp 
    chess_pos.cpp 
    chess_funcs.cpp 
    chess_moves_lut.cpp 
    node_move_list.cpp 
    ttable.cpp 
    bitops.cpp 
    postree.cpp
)

configure_file(k64config.h.in k64config.h)
add_executable(karly64 main.cpp uci.cpp)
target_link_libraries(karly64 PRIVATE Threads::Threads)
target_link_libraries(karly64 PRIVATE k64lib)
target_include_directories(karly64 PUBLIC
                           "${CMAKE_CURRENT_BINARY_DIR}"
                           )

set_target_properties(karly64 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


	

