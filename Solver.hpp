#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include "Position.hpp"
#include "Transposition.hpp"

class Solver {
public:
	unsigned long long nodeCount; // counter of explored nodes.
	int columnOrder[Position::WIDTH]; // column exploration order

	TranspositionTable T;

	Solver();

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


	int ply_score_to_move_score(int ply_score) {
		return (ply_score / 2) + (ply_score % 2); // division truncates towards zero, then we add/subtract 1 if ply_score is odd.
	}

	int32_t alpha_beta(Position& P);
	int negamax(Position &P, int alpha, int beta);

	// Go through files in a folder one at a time
	void test_file(std::string filename, std::ostream& strm);

};