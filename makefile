TARGET = karly64
SRC_DIR = ./src
CFLAGS = /EHsc /O2

all : test uciengine
uciengine : uciengine.exe
test : mct.exe

mct.exe uciengine.exe : chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj

uciengine.exe : uciengine.obj  search.obj	ttable.obj
	cl /Fe"./$(TARGET).exe" uciengine.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj search.obj ttable.obj

mct.exe : mct.obj 
	cl /Fe"./perft.exe" mct.obj chess_pos.obj chess_moves_lut.obj node_move_list.obj chess_funcs.obj


uciengine.obj : $(SRC_DIR)/uciengine.cpp  $(SRC_DIR)/constants.h  $(SRC_DIR)/chess_pos.h
	cl /c $(SRC_DIR)/uciengine.cpp $(CFLAGS) 
ttable.obj : $(SRC_DIR)/ttable.cpp  $(SRC_DIR)/constants.h
	cl /c $(SRC_DIR)/ttable.cpp $(CFLAGS) 
search.obj :  $(SRC_DIR)/search.cpp  $(SRC_DIR)/search.h  $(SRC_DIR)/constants.h  $(SRC_DIR)/chess_pos.h
	cl /c  $(SRC_DIR)/search.cpp $(CFLAGS) 
mct.obj :  $(SRC_DIR)/mct.cpp  $(SRC_DIR)/constants.h  $(SRC_DIR)/chess_pos.h
	cl /c  $(SRC_DIR)/mct.cpp $(CFLAGS) 
chess_pos.obj :  $(SRC_DIR)/chess_pos.cpp  $(SRC_DIR)/chess_pos.h  $(SRC_DIR)/constants.h
	cl /c  $(SRC_DIR)/chess_pos.cpp $(CFLAGS) 
chess_funcs.obj :  $(SRC_DIR)/chess_funcs.cpp  $(SRC_DIR)/chess_funcs.h  $(SRC_DIR)/constants.h
	cl /c  $(SRC_DIR)/chess_funcs.cpp $(CFLAGS) 
chess_moves_lut.obj :  $(SRC_DIR)/chess_moves_lut.cpp  $(SRC_DIR)/chess_moves_lut.h  $(SRC_DIR)/constants.h
	cl /c  $(SRC_DIR)/chess_moves_lut.cpp $(CFLAGS) 
node_move_list.obj :  $(SRC_DIR)/node_move_list.cpp  $(SRC_DIR)/node_move_list.h  $(SRC_DIR)/constants.h
	cl /c  $(SRC_DIR)/node_move_list.cpp $(CFLAGS) 

clean :
	del *.obj
	del *.exe