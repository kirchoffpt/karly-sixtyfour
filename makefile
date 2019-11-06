all : test play uciengine
uciengine : uciengine.exe
play : play.exe
test : mct.exe

uciengine.exe : uciengine.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj search.obj
	cl uciengine.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj search.obj

play.exe : play.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj
	cl play.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj

mct.exe : mct.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj
	cl mct.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj


uciengine.obj : uciengine.cpp constants.h chess_pos.h
	cl /c uciengine.cpp /EHsc /O2
search.obj : search.cpp search.h constants.h chess_pos.h
	cl /c search.cpp /EHsc /O2
play.obj : play.cpp constants.h chess_pos.h
	cl /c play.cpp /EHsc /O2
mct.obj : mct.cpp constants.h chess_pos.h
	cl /c mct.cpp /EHsc /O2
chess_pos.obj : chess_pos.cpp chess_pos.h constants.h
	cl /c chess_pos.cpp /EHsc /O2
chess_funcs.obj : chess_funcs.cpp chess_funcs.h constants.h
	cl /c chess_funcs.cpp /EHsc /O2
chess_moves_lut.obj : chess_moves_lut.cpp chess_moves_lut.h constants.h
	cl /c chess_moves_lut.cpp /EHsc /O2
node_move_list.obj : node_move_list.cpp node_move_list.h constants.h
	cl /c node_move_list.cpp /EHsc /O2

clean :
	del *.obj
	del *.exe