/***

 board representation
 6  13 20 ...    48
 5  12 19 ...    47 
 4  11 18 ...    46
 3  10 17        45
 2  9  16        44
 1  8  15        43
 0  7  14 ...    42


A position is represented by two boards: one for the current player, and one for both players

Ply Score: is +1 for winning on the last move, and increases by 1 for winning a ply earlier
Ply Score is -1 for losing on the last move and decreases by 1 for losing a ply earlier
***/

#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include "assert.h"

typedef uint64_t board;

class Position {
private:

public:
	static inline int n_positions_evaluated; // Counter for total number of positions created so far

	board current_mask;  // mask showing slots taken by current player
	board all_mask;		 // mask showing slots taken be either player
	int32_t nb_moves;
	Position() : current_mask(0), all_mask(0), nb_moves(0) {n_positions_evaluated++; }

	Position(Position &p) : current_mask(p.current_mask), all_mask(p.all_mask), nb_moves(p.nb_moves) {
		//std::cout << "Copying Position: \n";
		n_positions_evaluated++;
		//this->display();
	}
	
	// Constructor delegation
	Position(const std::string &moves) : Position() {
		play_moves_one_ind(moves);
	}

	static constexpr uint32_t HEIGHT = 6;
	static constexpr uint32_t WIDTH = 7;
	static constexpr uint32_t BOARD_SIZE = HEIGHT * WIDTH;

	static constexpr board BOARD_MASK() {
		return BOTTOM_MASK() * ((1LL << HEIGHT) - 1); // multiplication: {1<<X_i} * {1<<Y_j} = {1<< (X_i + Y_j) } for all combinations (i,j)
	}

	// bitmask with '1' in all the bottom spots
	static constexpr board BOTTOM_MASK() {
		board b = 0;
		for (int i = 0; i < WIDTH; ++i) {
			b |= static_cast<board>(1) << ( i * (HEIGHT+1) );
		}
		return b;
	};

	// bitmask with '1' in all the top spots
	static constexpr board TOP_MASK() {
		board b = 0;
		for (int i = 0; i < WIDTH; ++i) {
			b |= static_cast<board>(1) << ((HEIGHT - 1) + i * (HEIGHT+1) );
		}
		return b;
	};

	// Bitmask with top N rows set 
	static constexpr board TOP_N_MASK(int n) {
		board b = 0;
		for (int i = 0; i < n; ++i) {
			b |= TOP_MASK() >> i;
		}
		return b;
	};

	// Bitmask with bottom N rows set 
	static constexpr board BOTTOM_N_MASK(int n) {
		board b = 0;
		for (int i = 0; i < n; ++i) {
			b |= (TOP_MASK() >> (HEIGHT - 1) ) << i;
		}
		return b;
	};

	// Bitmask with rightmost N columns set
	static constexpr board RIGHT_N_MASK(int n) {
		board b = 0;
		for (int i = WIDTH - 1; i > WIDTH - 1 - n; --i) {
			b |= COL_MASK(0) << (i * (HEIGHT+1) );
		}
		return b;
	}

	static constexpr std::array<int, WIDTH> MOVE_ARRAY() {
		std::array<int, WIDTH> moves;
		for (int i = 0; i < WIDTH; ++i) {
			moves[i] = i;
		}
		return moves;
	}

	// bitmask with '1' in the bottom spot of a given column
	static constexpr board BOTTOM_MASK_COL(int col) {
		return static_cast<board>(1) << (col * (HEIGHT+1));
	};

	// bitmask with '1' in the top spot of a given column
	static constexpr board TOP_MASK_COL(int col) {
		return static_cast<board>(1) << (HEIGHT - 1 + (col * HEIGHT));
	};

	// bitmask with '1' in all spots in a given column
	static constexpr board COL_MASK(int col) {
		return ((static_cast<board>(1) << (HEIGHT)) - 1) << (col * (HEIGHT + 1));
	}

	// If game ends on the 42nd move, score is +1. If it ends one move before that, score is +2, etc...
	// Negative score if the end is a loss for the current player.
	int32_t SCORE() {return (HEIGHT * WIDTH) + 1 - nb_moves;}
	
	board key() {
		return current_mask + all_mask;
	}

	board get_mask(uint32_t row, uint32_t col) {
		assert(row <= HEIGHT - 1 && row >= 0);
		assert(col <= WIDTH - 1 && col >= 0);
		return ((board)1 << (row + col * (HEIGHT+1)));
	}

	board get_mask(uint32_t pos) {
		return (board)1 << pos;
	}


	// Legal moves are those above a placed stone or at the bottom row where there are no stones currently
	board get_legal() {
		return (all_mask + BOTTOM_MASK()) & BOARD_MASK();
	}

	// 'm' is a bitboard with only one bit set, and should be a legal move
	void play_move(board m) {
		current_mask |= m;
		all_mask |= m;
		current_mask = current_mask ^ all_mask;
		nb_moves++;
	}

	// all_mask + bottom mask is a bitmask with the first empty slot in each column set. 
	void play_col(int col) {
		play_move( (all_mask + BOTTOM_MASK_COL(col)) & COL_MASK(col));
	}

	void play_moves(std::string s) {
		for (char const& c : s) {
			// assert(is_legal_col(col));
			play_col( c - '0');
		}
	}

	// play moves given by columns indexed starting with '1' (to be consistent with online solver)
	void play_moves_one_ind(std::string s) {
		for (char const& c : s) {
			// assert(is_legal_col(col));
			play_col(c - '1');
		}
	}

	// return True if the last player who moved has connected 4 pieces
	bool gameover_zero() {
		board next_mask = current_mask ^ all_mask;
		board verti = next_mask & (next_mask << 1) & (next_mask << 2) & (next_mask << 3) & TOP_N_MASK(Position::HEIGHT - 3);
		board hori = next_mask & next_mask << (HEIGHT + 1) & next_mask << 2 * (HEIGHT + 1) & next_mask << 3 * (HEIGHT + 1) & RIGHT_N_MASK(WIDTH - 3);
		board diag1 = next_mask & next_mask << (HEIGHT + 2) & next_mask << (2 * (HEIGHT + 2)) & next_mask << (3 * (HEIGHT + 2)) & TOP_N_MASK(HEIGHT - 3);
		board diag2 = next_mask & next_mask << (HEIGHT)&next_mask << (2 * HEIGHT) & next_mask << (3 * HEIGHT) & BOTTOM_N_MASK(HEIGHT - 3);

		return ((verti | hori | diag1 | diag2) & BOARD_MASK()) != 0;
	}

	void display() {
		std::string s;
		for (int r = HEIGHT - 1; r >= 0; --r) {
			s.clear();
			for (int c = 0; c < WIDTH; ++c) {
				if (current_mask & get_mask(r, c)) { s += '1'; }
				else if (all_mask & get_mask(r, c)) { s += '2'; }
				else { s += "0"; }
			}
			std::cout << s << "\n";
		}
		std::cout << "\n";
	};

};