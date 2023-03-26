#pragma once
#include "Position.hpp"

class TranspositionTable {
private:
	// 4048831 8388617 32452867  67867979  122949829
	static constexpr uint64_t TABLE_SIZE = 8388617; // Observation: Larger tables run slower

public:
	// By Chinese Remainder Theorem, for any position P < 2^49
	// the pair: r1 = P % TABLE_SIZE and r2 = P % 2^32 correspond to a unique position P, as long as P <  TABLE_SIZE * 2^32 
	// Therefore, minimum table size is 2^17 = 131,072
	uint32_t* K; // Only last 32 bits kept from 64-bit key. 
	int8_t* V;

	TranspositionTable() {
		K = new uint32_t[TABLE_SIZE];
		V = new int8_t[TABLE_SIZE];

		memset(K, 0, TABLE_SIZE * sizeof(uint32_t));
		memset(V, 0, TABLE_SIZE * sizeof(int8_t));
	}

	~TranspositionTable() {
		delete[] K;
		delete[] V;
	}

	void put(uint64_t key, int val) {
		uint64_t ind = key % TABLE_SIZE;
		K[ind] = key; // key is truncated to 32 bits
		V[ind] = val;
	}

	int8_t get(uint64_t key) {
		uint64_t ind = key % TABLE_SIZE;
		if (K[ind] == (uint32_t)key) {
			return V[ind];
		}
		else return 0;
	}
};