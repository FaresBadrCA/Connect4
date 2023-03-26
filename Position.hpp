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

constexpr static uint64_t bottom(int width, int height) {
	return width == 0 ? 0 : bottom(width - 1, height) | 1LL << (width - 1) * (height + 1);
}

class Position {
private:

public:
	static inline int n_positions_evaluated; // Counter for total number of positions created so far
	static inline board threat_pair_masks[10]; // The 10 masks used to check for "win_in_3"

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

	static constexpr int32_t HEIGHT = 6;
	static constexpr int32_t WIDTH = 7;
	static constexpr int32_t BOARD_SIZE = HEIGHT * WIDTH;
	static const int MIN_SCORE = -(WIDTH * HEIGHT) / 2 + 3;
	static const int MAX_SCORE = (WIDTH * HEIGHT + 1) / 2 - 3;

	static constexpr int32_t offsets[3] = { HEIGHT, HEIGHT + 1, HEIGHT + 2 }; // Offsets to go diagonal (down slope), horizontal, and diagonal (up slope)

	// bitmask with '1' in the bottom spot of a given column
	static constexpr board BOTTOM_MASK_COL(int col) {
		return static_cast<board>(1) << (col * (HEIGHT+1));
	};

	// bitmask with '1' in all spots in a given column
	static constexpr board COL_MASK(int col) {
		return ((static_cast<board>(1) << (HEIGHT)) - 1) << (col * (HEIGHT + 1));
	}

	int moveScore(uint64_t move) const {
		return popcount(get_threats(current_mask | move) & (BOARD_MASK ^ all_mask));
	}


	board key() {
		return current_mask + all_mask;
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
	/*
	bool gameover_zero() {
		board next_mask = current_mask ^ all_mask;
		board verti = next_mask & (next_mask << 1) & (next_mask << 2) & (next_mask << 3) & TOP_N_MASK(Position::HEIGHT - 3);
		board hori = next_mask & next_mask << (HEIGHT + 1) & next_mask << 2 * (HEIGHT + 1) & next_mask << 3 * (HEIGHT + 1) & RIGHT_N_MASK(WIDTH - 3);
		board diag1 = next_mask & next_mask << (HEIGHT + 2) & next_mask << (2 * (HEIGHT + 2)) & next_mask << (3 * (HEIGHT + 2)) & TOP_N_MASK(HEIGHT - 3);
		board diag2 = next_mask & next_mask << (HEIGHT)&next_mask << (2 * HEIGHT) & next_mask << (3 * HEIGHT) & BOTTOM_N_MASK(HEIGHT - 3);

		return ((verti | hori | diag1 | diag2) & BOARD_MASK()) != 0;
	}
	*/

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
		board opponent_threats = get_threats(current_mask ^ all_mask) & BOARD_MASK; // Threats on the board
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

	// Legal moves are those above a placed stone or at the bottom row where there are no stones currently
	uint64_t get_legal() const {
		return (all_mask + BOTTOM_MASK) & BOARD_MASK;
	}

	const static uint64_t BOTTOM_MASK = bottom(WIDTH, HEIGHT);
	const static uint64_t BOARD_MASK = BOTTOM_MASK * ((1LL << HEIGHT) - 1);

	// 1 -> spot taken by current player
	// 0 -> spot that is empty
	// L -> spot that is legal to play
	// 
	// PART 1: double threats - That is two threat-generating pairs that share a spot
	// A threat-generating pair for example is: 11LL. If any 'L' is played, we have a threat in the remaining 'L'. 
	//
	// Step 1: Create bitmasks of threat-generating pairs, pairs separated by one spot, 2 spots, or 3 spots:
	// sep1 group: LL11 -> 0100, 1LL1 -> 0010, 11LL -> 0001 
	// sep2 group: L1L1 -> 0010, 1L1L -> 0001
	// sep3 group: L11L -> 0001
	//
	// Step 2: Get 3 bitmasks from those 6 as follows: sep1 = disjunction of all masks in sep1 group. sep2 = disjunction of all masks in sep2 group.
	// Reason: if we skip to the step where we find bits set in more than one mask, we would mark 2 pairs that are identical
	// For example: 110011 will be present in the 11LL and LL11 masks, but it only counts as one threat pair, so not a double threat.
	// That is why we compress those 6 masks into 3 masks, so we do not double-count the same threat-generating pair
	// 
	// Step 3: Change the sep masks to have all threat-generating spots set to 1:
	// sep1 |= sep1 left shift by 1.	sep2 |= sep2 left shift by 2	sep3 |= sep3 shift by 3 
	//
	// Step 4: do steps 1 and 2 for horizontal, diagonal1 and diagonal2 threats to produce 9 masks (sep1/2/3 3 times)
	// Step 5: produce one more mask: 11L0 -> 0010 vertical threat generator
	// Step 6: Find any bit that is set in more than one mask. That is a double threat-generating move.
	
	// PART 2: win by threat-generating pair below a threat
	/*
	board win_in_3() {
		board legal = get_legal();
		board empty = BOARD_MASK & ~all_mask;
		board opponent_immediate_threats = get_threats(current_mask ^ all_mask) & legal;

		if (opponent_immediate_threats) {
			legal &= opponent_immediate_threats; // The only move we have is to block the threat
		}

		for (int i = 0; i < 3; ++i) {
			uint32_t offset = offsets[i];
			board temp = legal & legal << offset;
			threat_pair_masks[3 * i + 0] = // sep1
				temp & (current_mask >> offset) & (current_mask >> 2 * offset) |    // LL11 -> 0100
				temp & (current_mask << 2 * offset) & (current_mask >> offset) |    // 1LL1 -> 0010
				temp & (current_mask << 2 * offset) & (current_mask << 3 * offset); // 11LL -> 0001
			threat_pair_masks[3 * i + 0] |= threat_pair_masks[3 * i + 0] >> offset;

			temp = legal & legal << 2 * offset;
			threat_pair_masks[3 * i + 1] = // sep2
				temp & (current_mask << offset) & (current_mask >> offset) |  // L1L1 -> 0010
				temp & (current_mask << offset) & (current_mask << 3 * offset); // 1L1L -> 0001
			threat_pair_masks[3 * i + 1] |= threat_pair_masks[3 * i + 1] >> 2 * offset;

			threat_pair_masks[3 * i + 2] = legal & legal << 3 * offset & current_mask << offset & current_mask << 2 * offset; // sep3: L11L -> 0001
			threat_pair_masks[3 * i + 2] |= threat_pair_masks[3 * i + 2] >> 3 * offset;
		}
		threat_pair_masks[9] = current_mask << 2 & current_mask << 1 & legal & empty >> 1; // Vertical 11L0 -> 0010

		board threat_generating_masks = 0; 
		board result = 0;
		for (int i = 0; i < 9; ++i) {
			result |= (threat_generating_masks & threat_pair_masks[i]);
			threat_generating_masks |= threat_pair_masks[i];
		}

		// PART 2: win by threat-generating pair below a threat, excluding vertical threat-generator
		board threats = get_threats(current_mask) & empty;
		result |= (threats >> 1) & threat_generating_masks;

		// We skipped vertical threat-generators earlier, so we consider them now
		result |= (threat_generating_masks & threat_pair_masks[9]);
		threat_generating_masks |= threat_pair_masks[9];

		// PART 3: Win if there is a threat, and there is only one empty spot left that is not directly below a threat.
		// Additionally,
		board active_threats = empty & threats;
		if (active_threats) {
			board playable_spots = ( (active_threats | EXTRA_ROW_MASK()) - BOTTOM_MASK) ^ active_threats; // bitmask with the threat and all spots below it
			playable_spots &= empty & ~(active_threats | active_threats >> 1); // exclude the threat and the spot below it.
			if (!(playable_spots & (playable_spots - 1))) { // if there is only one spot left

				if (opponent_immediate_threats) {
					playable_spots &= opponent_immediate_threats; // must block the immediate threat
				}

				result |= playable_spots; // then that spot is win-in-3
			}
		}
		return result;
 	}
	*/
	static uint8_t popcount(board mask) {
		#ifndef _MSC_VER
		#error "__popcount64 only defined for MSVC compiler"
		#endif
		return __popcnt64(mask);
	}

	// Return the number of threats the opponent has. Includes threats that cannot be immediately played.
	int count_opponent_threats() {
		return popcount( get_threats(current_mask ^ all_mask) & BOARD_MASK & ~all_mask );
	}


	//******* DISPLAY FUNCTIONS ********* //

	board get_mask(uint32_t row, uint32_t col) {
		assert(row <= HEIGHT - 1 && row >= 0);
		assert(col <= WIDTH - 1 && col >= 0);
		return ((board)1 << (row + col * (HEIGHT + 1)));
	}

	board get_mask(uint32_t pos) {
		return (board)1 << pos;
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

	void display_board(board b) {
		std::string s;
		for (int r = Position::HEIGHT - 1; r >= 0; --r) {
			s.clear();
			for (int c = 0; c < Position::WIDTH; ++c) {
				if (b & get_mask(r, c)) { s += "X"; }
				else { s += "-"; }
			}
			std::cout << s << "\n";
		}
		std::cout << "\n";
	}

	void display_board_with_extra_row(board b) {
		std::string s;
		for (int r = Position::HEIGHT; r >= 0; --r) {
			s.clear();
			for (int c = 0; c < Position::WIDTH; ++c) {
				board mask = ((board)1 << (r + c * (Position::HEIGHT + 1)));
				if (b & mask) { s += "X"; }
				else { s += "-"; }
			}
			std::cout << s << "\n";
		}
		std::cout << "\n";
	}

};