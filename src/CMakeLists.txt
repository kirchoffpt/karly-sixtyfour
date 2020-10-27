

add_executable(karly64 uciengine.cpp search.cpp chess_pos.cpp chess_funcs.cpp chess_moves_lut.cpp node_move_list.cpp ttable.cpp bitops.cpp)
target_link_libraries(karly64 PRIVATE Threads::Threads)

add_executable(perft mct.cpp search.cpp chess_pos.cpp chess_funcs.cpp chess_moves_lut.cpp node_move_list.cpp ttable.cpp bitops.cpp)



