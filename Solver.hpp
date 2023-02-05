#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include "Position.hpp"
#include "Transposition.hpp"

// returning from a depth-limited negamax returns a window of possible values
struct Window {
	int alpha;
	int beta;
};

class Solver {
public:
	TranspositionTable T;
	bool depth_limited = true; // Change to false when deeper search will not change evaluation, so search is complete
	int max_depth = 1; 

	board get_mask(uint32_t row, uint32_t col) {
		assert(row <= Position::HEIGHT - 1 && row >= 0);
		assert(col <= Position::WIDTH - 1 && col >= 0);
		return ((board)1 << (row + col * (Position::HEIGHT + 1)));
	}

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

	int ply_score_to_move_score(int ply_score) {
		return (ply_score / 2) + (ply_score % 2);
	}

	int32_t alpha_beta(Position& P);
	Window negamax(Position &P, int alpha, int beta, int depth);

	// Go through files in a folder one at a time
	void test_file(std::string filename, std::ostream& strm);

};