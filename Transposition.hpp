#pragma once
#include "Position.hpp"

class TranspositionTable {
private:
	// 32452867  67867979  122949829
	static constexpr uint64_t TABLE_SIZE = 67867979;

public:
	std::array<uint64_t, TABLE_SIZE>* K; // heap-allocated
	std::array<int8_t, TABLE_SIZE>* V_a; // alpha
	std::array<int8_t, TABLE_SIZE>* V_b; // beta

	TranspositionTable() {
		K = new std::array<uint64_t, TABLE_SIZE>;
		V_a = new std::array<int8_t, TABLE_SIZE>;
		V_b = new std::array<int8_t, TABLE_SIZE>;
		K->fill(0); // We never have a key of zero, so this value indicated no key in that slot
	}

	~TranspositionTable() {
		delete K;
		delete V_a;
		delete V_b;
	}

	void put(uint64_t key, int alpha, int beta) {
		// std::cout << "Write key: " << key << " With alpha/beta: " << alpha << "/" << beta << "\n";
		uint64_t ind = key % TABLE_SIZE;
		K->at(ind) = key;
		V_a->at(ind) = alpha;
		V_b->at(ind) = beta;
	}

	bool get(uint64_t key, int& alpha, int& beta) {
		uint64_t ind = key % TABLE_SIZE;
		if (K->at(ind) == key) {
			alpha = V_a->at(ind);
			beta = V_b->at(ind);
			return true;
		}
		return false;
	}

};