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
	board get_legal() const {
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

	// return a bitmask with moves that win in 1 ply
	board winning_moves() const {
		return get_threats(current_mask) & get_legal();
	}

	// Return a bitmask with spots which would make 4 in a row if any one is filled. INCLUDES NON-REAL THREATS IN THE EXTRA ROW
	board get_threats(board mask) const {
		board result = 0;
		result |= (mask & mask << 1 & mask << 2) << 1; // Vertical -111

		board temp = mask & mask << HEIGHT + 1; // Horizontal --11 has bits set in a '1' spot with a '1' to its left
		result |= (temp & mask << 2 * (HEIGHT + 1)) << (HEIGHT + 1); // 111-
		result |= (temp & mask << 2 * (HEIGHT + 1)) >> 3 * (HEIGHT + 1); // -111
		result |= (temp & mask << 3 * (HEIGHT + 1)) >> 2 * (HEIGHT + 1); // 1-11
		result |= (mask & temp << 2 * (HEIGHT + 1)) >> (HEIGHT + 1); // 11-1

		temp = mask & mask << HEIGHT + 2; // Diagonal 1: right-up
		result |= (temp & mask << 2 * (HEIGHT + 2)) << (HEIGHT + 2); // Diagonal 111-
		result |= (temp & mask << 2 * (HEIGHT + 2)) >> 3 * (HEIGHT + 2); // Diagonal -111
		result |= (temp & mask << 3 * (HEIGHT + 2)) >> 2 * (HEIGHT + 2); // Diagonal 1-11
		result |= (mask & temp << 2 * (HEIGHT + 2)) >> (HEIGHT + 2); // Diagonal 11-1

		temp = mask & mask << HEIGHT; // Diagonal 2: right-down
		result |= (temp & mask << 2 * (HEIGHT)) << (HEIGHT); // Diagonal 111-
		result |= (temp & mask << 2 * (HEIGHT)) >> 3 * (HEIGHT); // Diagonal -111
		result |= (temp & mask << 3 * (HEIGHT)) >> 2 * (HEIGHT); // Diagonal 1-11
		result |= (mask & temp << 2 * (HEIGHT)) >> (HEIGHT); // Diagonal 11-1

		return result;
	}


	// Return bitmask with moves that will not lose within 2 plies. Assumes there are no immediately winning moves.
	board nonlosing_moves() {
		board opponent_threats = get_threats(current_mask ^ all_mask) & BOARD_MASK(); // Threats on the board
		board possible = get_legal();
		board forced_moves = opponent_threats & possible;

		if (forced_moves) {
			if (forced_moves & (forced_moves - 1)) { // 2 or more bits are set
				return 0; // 2 or more concurrent threats, so nothing can be played to not lose.
			}
			else possible = forced_moves; // only possible move is to block the threat
		}
		return possible & ~(opponent_threats >> 1); // Legal moves not below a threat
	}

	static uint8_t popcount(board mask) {
		#ifndef _MSC_VER
		#error "__popcount64 only defined for MSVC compiler"
		#endif
		return __popcnt64(mask);
	}

	// For move ordering: The more threats a move creates, the higher its priority.
	int get_move_priority(board move) const {
		return popcount( get_threats( current_mask | move) & BOARD_MASK() & ~all_mask );
	}

	// Return the number of threats the opponent has. Includes threats that cannot be immediately played.
	int count_opponent_threats() {
		return popcount( get_threats(current_mask ^ all_mask) & BOARD_MASK() & ~all_mask );
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