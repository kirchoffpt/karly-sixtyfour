CFLAGS = /EHsc /O2

all : test uciengine
uciengine : uciengine.exe
test : mct.exe

mct.exe uciengine.exe : chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj

uciengine.exe : uciengine.obj  search.obj
	cl uciengine.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj search.obj

mct.exe : mct.obj 
	cl mct.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj


uciengine.obj : uciengine.cpp constants.h chess_pos.h
	cl /c uciengine.cpp $(CFLAGS) 
search.obj : search.cpp search.h constants.h chess_pos.h
	cl /c search.cpp $(CFLAGS) 
play.obj : play.cpp constants.h chess_pos.h
	cl /c play.cpp $(CFLAGS) 
mct.obj : mct.cpp constants.h chess_pos.h
	cl /c mct.cpp $(CFLAGS) 
chess_pos.obj : chess_pos.cpp chess_pos.h constants.h
	cl /c chess_pos.cpp $(CFLAGS) 
chess_funcs.obj : chess_funcs.cpp chess_funcs.h constants.h
	cl /c chess_funcs.cpp $(CFLAGS) 
chess_moves_lut.obj : chess_moves_lut.cpp chess_moves_lut.h constants.h
	cl /c chess_moves_lut.cpp $(CFLAGS) 
node_move_list.obj : node_move_list.cpp node_move_list.h constants.h
	cl /c node_move_list.cpp $(CFLAGS) 

clean :
	del *.obj
	del *.exe