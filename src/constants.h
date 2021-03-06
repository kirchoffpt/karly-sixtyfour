#define LOG_UCI_INPUT 1
#define FILEOUT "uci_input_log.txt"
#define MAX_MOVE_TIME_USAGE 0.25

//DEBUG 1- fast assertions 
//DEBUG 2- previously used for move generation debugging

#define EVAL_GRAIN	4
#define MAX_AB_DEPTH 64
#define MAX_Q_DEPTH 64 
#define Q_DELTA_WINDOW 256
#define P_MAT	128
#define N_MAT	782
#define B_MAT	830
#define R_MAT	1289
#define Q_MAT	2529

#define CLEAR_TTABLE_BEFORE_SEARCH false 
#define EXCLUDE_PAWNS_FROM_CAPTURE_MASK false
#define USE_OLD_TTABLE_BEST_MOVES true 			//works fine but alters timings for tests/comparisons
#define ENABLE_NULL_MOVE_PRUNING true 			//enable null move pruning

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

//settings for correspondence mode
#define CORRESPONDENCE_MODE_THRESHOLD 86400E3 //time per move (ms) greater/equal than this will enable correspondence mode
#define CSPOND_TIME_BASE 1E3
#define CSPOND_TIME_INCREMENT 3E3 //gets added to base time if score is not sufficient 
#define CSPOND_TIME_DECAY 0.99 //max time is base+sum(incr*decay^n,n,0,infinity)
#define CSPOND_TRGT_DEPTHSCORE 5000 //check implementation in search.cpp for reference
#define CSPOND_CONTEMPT 35 //centipawns added to top score when checking to see if search should be extended

////////////////////////////////////////////
//below values should not be changed lightly
////////////////////////////////////////////

#define DEBUG 0 //deprecated

typedef unsigned long long z_key;

#define MAX_DEPTH MAX_AB_DEPTH+MAX_Q_DEPTH

#define TOTAL_MAT P_MAT*16+N_MAT*4+B_MAT*4+R_MAT*4+Q_MAT*2

#define MAX_PIECES_PER_SIDE 16
#define MAX_MOVES_IN_GAME 256
#define MAX_MOVES_IN_POS 256 //218 maxiumum moves in a position

#define CHECKMATE 1E9 //score is this number minus the ply
#define MATE_BUFFER 1E6 //every non mate score should fall under CHECKMATE-MATE_BUFFER
#define STALEMATE 0

#define F_HI 	1E38
#define F_LO 	-1E38

#define SCORE_HI 	1E9
#define SCORE_LO 	-1E9

//16 bit move, SRC-DST-SPECIAL-PROMOPIECE
#define SRC_MASK		0xFC00
#define DST_MASK		0x03F0
#define SPECIAL_MASK	0x000C
#define PROMO_MASK		0x0003
#define SRC_SHIFT 		10
#define DST_SHIFT		4

#define PROMOTION_RANKS	0xFF000000000000FF
#define EN_PASSANT_ATTACKER_RANKS 0x000000FFFF000000
#define PAWN_RANK_WHITE 0x000000000000FF00
#define PAWN_RANK_BLACK 0x00FF000000000000

#define RANK_8 0xFF00000000000000
#define RANK_7 0x00FF000000000000
#define RANK_6 0x0000FF0000000000
#define RANK_5 0x000000FF00000000
#define RANK_4 0x00000000FF000000
#define RANK_3 0x0000000000FF0000
#define RANK_2 0x000000000000FF00
#define RANK_1 0x00000000000000FF

#define A_FILE (H_FILE << 7)
#define B_FILE (H_FILE << 6)
#define C_FILE (H_FILE << 5)
#define D_FILE (H_FILE << 4)
#define E_FILE (H_FILE << 3)
#define F_FILE (H_FILE << 2)
#define G_FILE (H_FILE << 1)
#define H_FILE 0x0101010101010101

#define CENTER 0x00003C3C3C3C0000

#define LIGHT_SQUARES 0xAA55AA55AA55AA55
#define DARK_SQUARES ~LIGHT_SQUARES

//SPECIAL
#define PROMO 	0x0004
#define ENPAS	0x0008
#define CASTL	0x000C

//PROMOPIECE
#define N_PROMO 0x0000
#define B_PROMO	0x0001
#define R_PROMO	0x0002
#define Q_PROMO	0x0003
#define PROMO_TO_PIECE 1 //add this to numubers below to get piece type from promo

#define PAWN 		0
#define W_PAWN 		0
#define KNIGHT 		1
#define BISHOP		2
#define ROOK		3
#define QUEEN		4
#define KING 		5
#define B_PAWN 		6

#define WHITE 		0
#define BLACK 		1

#define KING_SIDE_CASTLE_WHITE  8
#define QUEEN_SIDE_CASTLE_WHITE 4
#define KING_SIDE_CASTLE_BLACK  2
#define QUEEN_SIDE_CASTLE_BLACK 1

#define KSCW_CHECK  0x0000000000000006
#define QSCW_CHECK  0x0000000000000030
#define KSCB_CHECK  0x0600000000000000
#define QSCB_CHECK  0x3000000000000000

#define KSCW_CLEAR  0x0000000000000006
#define QSCW_CLEAR  0x0000000000000070
#define KSCB_CLEAR  0x0600000000000000
#define QSCB_CLEAR  0x7000000000000000

#define KSCW_KING_SQUARE  0x0000000000000002
#define QSCW_KING_SQUARE  0x0000000000000020
#define KSCB_KING_SQUARE  0x0200000000000000
#define QSCB_KING_SQUARE  0x2000000000000000
#define KSCW_KING_IDX  1
#define QSCW_KING_IDX  5
#define KSCB_KING_IDX  57
#define QSCB_KING_IDX  61

#define KSCW_ROOK_SQUARE  0x0000000000000004
#define QSCW_ROOK_SQUARE  0x0000000000000010
#define KSCB_ROOK_SQUARE  0x0400000000000000
#define QSCB_ROOK_SQUARE  0x1000000000000000
#define KSCW_ROOK_IDX  2
#define QSCW_ROOK_IDX  4
#define KSCB_ROOK_IDX  58
#define QSCB_ROOK_IDX  60

#define KSCW_CHANGED_SQUARES  0x000000000000000F
#define QSCW_CHANGED_SQUARES  0x00000000000000B8
#define KSCB_CHANGED_SQUARES  0x0F00000000000000
#define QSCB_CHANGED_SQUARES  0xB800000000000000

#define E_DIR	0
#define SE_DIR	1
#define S_DIR 	2
#define SW_DIR	3
#define W_DIR	4
#define NW_DIR	5
#define N_DIR 	6
#define NE_DIR	7 

#define PVNODE 2 
#define CUTNODE 1
#define ALLNODE 0	
