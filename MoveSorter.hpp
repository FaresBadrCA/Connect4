#pragma once

#include <queue>
#include <algorithm>
#include "Position.hpp"


struct ScoredMove {
	board move = 0; // bitmask with move to play
	int priority_value = 0;
	
	ScoredMove(board mask, int val) {
		move = mask;
		priority_value = val;
	};
};

// Take in a bitmask of possible moves and produce an ordered list of positions
class MoveSorter {
public:
	std::vector<ScoredMove> moves_arr;
	MoveSorter(const Position& P, board moves) {
		moves_arr.reserve(Position::WIDTH);
		for (int i = 0; i < Position::WIDTH; ++i) {
			board move = moves & Position::COL_MASK(Position::MOVE_ARRAY()[i]);
			if (move) {
				moves_arr.push_back(ScoredMove(move, P.get_move_priority(move)));
			}
		}
		std::sort(moves_arr.begin(), moves_arr.end(), 
			[](const ScoredMove& m1, const ScoredMove& m2) {return m1.priority_value > m2.priority_value; }); // Want to sort in descending order. ">" operator achieves this.
	}
};