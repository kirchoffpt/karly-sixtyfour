#include "chess_moves_lut.h"


#define INCLUDE_CENTER false

const ll ray_NwSe = 0x8040201008040201; // '\'
const ll ray_SwNe = 0x0102040810204080; // '/'
const ll ray_SeSw = 0x00000000000000FF;
const ll ray_NeNw = 0xFF00000000000000;
const ll ray_NwSw = 0x8080808080808080;
const ll ray_NeSe = 0x0101010101010101;

const ll ray_SeNw = ray_NwSe;
const ll ray_NeSw = ray_SwNe;
const ll ray_SwSe = ray_SeSw;
const ll ray_NwNe = ray_NeNw;
const ll ray_SwNw = ray_NwSw;
const ll ray_SeNe = ray_NeSe;

const ll knight_pattern_18 = 0x0000000A1100110A; //centered at idx 18
const ll king_patt_9 = 0x70507;

const int const_piece_square_pawn[64] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
	 5,  5, 10, 25, 25, 10,  5,  5,
	 0,  0,  0, 20, 20,  0,  0,  0,
	 5, -5,-10, 10, 10,-10, -5,  5,
	 5, 10, 10,-20,-20, 10, 10,  5,
	 0,  0,  0,  0,  0,  0,  0,  0
};

const int const_piece_square_king[64] = {
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	 20, 20,  0,  0,  0,  0, 20, 20,
	 20, 30, 10,  0,  0, 10, 30, 20
};

using namespace std;

ll reverse_U64(ll x){ //slow
	ll y = 0;
	int i;

	for(i=0;i<64;i++){
		y |= (((x>>i)&ll(1)) << (63-i));
	}

	return y;
}

chess_mask_LUT::chess_mask_LUT() {

	int i,j,k,x,y,z;
	int shift;
	ll temp = 0, temp2, ab;

	//PAWN ATTACK WHITE
	for(i=0;i<64;i++){
		temp = 0;
		if(i < 56){
			temp |= ll(0xA) << (i+6);
			if(i%8 == 0) {
				temp &= ~ray_NwSw;
			} else if(i%8 == 7){
				temp &= ~ray_NeSe;
			}
		}


		pawn_attack_mask[WHITE][i] = temp;
	}

	//PAWN ATTACK BLACK
	for(i=0;i<64;i++){
		pawn_attack_mask[BLACK][i] = reverse_U64(pawn_attack_mask[WHITE][63-i]);
	}


	//PAWN WHITE
	for(i=0;i<64;i++){
		temp = 0;
		if(i < 56){
			temp |= ll(1) << (i+8);
			if(i%8 == 0) {
				temp &= ~ray_NwSw;
			} else if(i%8 == 7){
				temp &= ~ray_NeSe;
			}
		}
		if((i < 16)&&(i > 7)) temp |= (ll(1) << (i+16));


		move_mask[W_PAWN][i] = temp;
	}

	//PAWN BLACK
	for(i=0;i<64;i++){
		move_mask[B_PAWN][i] = reverse_U64(move_mask[W_PAWN][63-i]);
	}

	//KNIGHT
	for(i=0;i<64;i++){
		if(i<=18){
			temp = (knight_pattern_18 << i) >> 18;
		}else{
			temp = (knight_pattern_18 << (i-18));
		}
		if(i%8 >= 6){
			temp &= ~ray_NeSe;
			temp &= ~(ray_NeSe<<1);
		} else if(i%8 <= 1){
			temp &= ~ray_NwSw;
			temp &= ~(ray_NwSw>>1);
		}
		if(INCLUDE_CENTER) temp |= ll(1) << i;
		move_mask[KNIGHT][i] = temp;
	}

	//BISHOP
	for(i=0;i<64;i++){
		temp = 0;
		shift = (i%8 - i/8);
		if(shift >= 0){
			temp |= ray_NwSe >> 8*(shift);
		} else {
			temp |= ray_NwSe << 8*(-shift);
		}

		shift = (i%8 - (7-i/8));
		if(shift >= 0){
			temp |= ray_NeSw << 8*(shift);
		} else {
			temp |= ray_NeSw >> 8*(-shift);
		}
		if(!INCLUDE_CENTER) temp &= ~(ll(1) << i);
		move_mask[BISHOP][i] = temp;
	}

	//ROOK
	for(i=0;i<64;i++){
		temp = 0;
		temp |= ray_NeSe << i%8;
		temp |= ray_SeSw << 8*(i/8);

		if(!INCLUDE_CENTER) temp &= ~(ll(1) << i);
		move_mask[ROOK][i] = temp;
	}

	//QUEEN
	for(i=0;i<64;i++){
		move_mask[QUEEN][i] = move_mask[BISHOP][i] | move_mask[ROOK][i];
	}

	//KING
	for(i=0;i<64;i++){
		temp = 0;
		if(i < 32){
			temp |= (ll(king_patt_9) << i) >> 9;
		} else {
			temp |= (ll(king_patt_9) << (i-9));
		}
		if(i%8 == 0) {
			temp &= ~ray_NwSw;
		} else if(i%8 == 7){
			temp &= ~ray_NeSe;
		}
		if(!INCLUDE_CENTER) temp &= ~(ll(1) << i);
		move_mask[KING][i] = temp;
	}	
	
	//V_MASK
	for(i=0;i<64;i++){
		temp = move_mask[BISHOP][i];
		for(j=0;j<i/8;j++){
			temp &= ~(ray_SeSw << (8*j));
		}
		temp |= (ll(1) << i);
		V_mask[0][i] = temp;
	}

	//V_MASK_REVERSE
	for(i=0;i<64;i++){
		temp = move_mask[BISHOP][i] ^ V_mask[0][i];
		temp |= (ll(1) << i);
		V_mask[1][i] = temp;
	}

	//L_MASK_REVERSE
	for(i=0;i<64;i++){
		temp = move_mask[ROOK][i];
		temp &= ~(ray_SeSw << i);
		temp &= ~(ray_NeSe << i);
		temp |= (ll(1) << i);
		L_mask[1][i] = temp;
	}

	//L_MASK
	for(i=0;i<64;i++){
		temp = move_mask[ROOK][i] ^ L_mask[1][i];
		temp |= (ll(1) << i);
		L_mask[0][i] = temp;
	}

	//SLIDING RAYS
	for (i=0; i < 64; i++)
	{
		for (j=0;j<i; j++)
		{
			temp = (ll(1) << i)|(ll(1) << j);
			ab = temp;
			x = (i%8)-(j%8);
			y = (i/8)-(j/8);

			if(x == 0){
				k = 8;
			} else if(y == 0){
				k =	1;	
			} else if(x == y){
				k = 9;
			} else if(x == -y){
				k = 7;
			} else {
				k = 0;
			}
			if(k != 0){
				for(z = i-k; z > j; z -= k){
					temp |= (ll(1) << z);
				}
				sliding_rays.insert(std::make_pair(ab, temp));
			}
		}
	}

	// DIAG RAYS
		//SE, SW, NW, NE
	for (i=0; i < 64; i++)
	{
		//SE 
		temp = ray_NwSe;
		temp = temp << 8*(7-i%8);
		temp = temp >> (8*(7-i/8));
		//temp &= ~(ll(1) << i);
		diag_ray[0][i] = temp;

		//SW
		temp = ray_SwNe;
		temp = temp << 8*(i%8);
		temp = temp >> (8*(7-i/8));
		//temp &= ~(ll(1) << i);
		diag_ray[1][i] = temp;

		//NW
		temp = ray_NwSe;
		temp = temp >> (8*(i%8));
		temp = temp << 8*(i/8);
		//temp &= ~(ll(1) << i);
		diag_ray[2][i] = temp;

		//NE
		temp = ray_NeSw;
		temp = temp >> 8*(7-i%8);
		temp = temp << 8*(i/8);
		//temp &= ~(ll(1) << i);
		diag_ray[3][i] = temp;
	}


	// STRAIGHT RAYS
		//right, bottom, left, top
	for (i=0; i < 64; i++)
	{
		//E 
		temp = ray_SwSe;
		temp = temp >> (7-i%8);
		temp = temp << (8*(i/8));
		//temp &= ~(ll(1) << i);
		straight_ray[0][i] = temp;

		//S
		temp = ray_NeSe;
		temp = temp << i%8;
		temp = temp >> (8*(7-i/8));
		//temp &= ~(ll(1) << i);
		straight_ray[1][i] = temp;

		//W
		temp = straight_ray[0][i] ^ (ray_SwSe << 8*(i/8));
		//temp &= ~(ll(1) << i);
		temp |= ll(1) << i;
		straight_ray[2][i] = temp;

		//N
		temp = straight_ray[1][i] ^ (ray_NeSe << (i%8));
		//temp &= ~(ll(1) << i);
		temp |= ll(1) << i;
		straight_ray[3][i] = temp;
	}

	//PAWN AREA OF INFLUENCES
	for (i=0; i < 64; i++)
	{
		temp = temp2 = 0;
		k = (i/8)+2;
		if(k > 7) k = 7;
		for(j=i/8;j<=k;j++){
			temp |= ray_SwSe << 8*(j);
		}
		j = (i%8)-2;
		k = (i%8)+2;
		if(j < 0) j = 0;
		if(k > 7) k = 7;
		for(j;j<=k;j++){
			temp2 |= ray_NeSe << j;
		}
		pawn_area_of_influence[0][i] = temp & temp2;
		pawn_area_of_influence[1][63-i] = reverse_U64(pawn_area_of_influence[0][i]);
	}
	//ROOK AREA OF INFLUENCES
	for (i=0; i < 64; i++)
	{
		temp = 0;
		j = (i/8)-1;
		k = (i/8)+1;
		if(k > 7) k = 7;
		if(j < 0) j = 0;
		for(j;j<=k;j++){
			temp |= ray_SwSe << 8*(j);
		}
		j = (i%8)-1;
		k = (i%8)+1;
		if(j < 0) j = 0;
		if(k > 7) k = 7;
		for(j;j<=k;j++){
			temp |= ray_NeSe << j;
		}
		rook_area_of_influence[i] = temp;
	}
	//EN PASSANT ATTACKERS
	for (i=0; i < 64; i++)
	{
		temp = 0;
		if((i < 48) && (i >= 40)){
			temp = pawn_attack_mask[0][i-16];
		}
		if((i < 24) && (i >= 16)){
			temp = pawn_attack_mask[1][i+16];
		}
		en_passant_attackers[i] = temp;
	}

	//PIECE SQUARE TABLES
	for (i=0; i < 64; i++)
	{
		piece_square_pawn[i] = const_piece_square_pawn[63-i];
		piece_square_king[i] = const_piece_square_king[63-i];
	}

}

ll chess_mask_LUT::get_move_mask(int piece_type, int piece_position)
{
	return move_mask[piece_type][piece_position];
}

ll chess_mask_LUT::get_pawn_attack_mask(int side, int piece_position)
{
	return pawn_attack_mask[side][piece_position];
}

ll chess_mask_LUT::getVmask(bool reversed, int pos)
{
	return V_mask[reversed][pos];
}

ll chess_mask_LUT::getLmask(bool reversed, int pos)
{
	return L_mask[reversed][pos];
}

ll chess_mask_LUT::get_sliding_ray(ll AB)
{
	return sliding_rays[AB];
}

ll chess_mask_LUT::get_diag_ray(int direction, int index) //SE, SW, NW, NE
{
	return diag_ray[direction][index];
}

ll chess_mask_LUT::get_straight_ray(int direction, int index) //right, bottom, left, top
{
	return straight_ray[direction][index];
}

ll chess_mask_LUT::get_pawn_area_of_influence(int side, int index)
{
	return pawn_area_of_influence[side][index];
}

ll chess_mask_LUT::get_rook_area_of_influence(int index)
{
	return rook_area_of_influence[index];
}

ll chess_mask_LUT::get_en_passant_attackers(int index)
{
	return en_passant_attackers[index];
}

int chess_mask_LUT::get_piece_square_pawn(int index)
{
	return piece_square_pawn[index];
}

int chess_mask_LUT::get_piece_square_king(int index)
{
	return piece_square_king[index];
}
