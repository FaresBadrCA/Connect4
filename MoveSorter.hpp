#pragma once

#include "Position.hpp"

/**
* This class helps sorting the next moves
*
* You have to add moves first with their score
* then you can get them back in decreasing score
*
* This class implement an insertion sort that is in practice very
* efficient for small number of move to sort (max is Position::WIDTH)
* and also efficient if the move are pushed in approximatively increasing
* order which can be acheived by using a simpler column ordering heuristic.
*/
class MoveSorter {
private:
    unsigned int size; // number of stored moves

    struct {
        board move;
        int score;
    } entries[Position::WIDTH];


public:

    void add(const board move, const int score) {
        int pos = size++;
        for (; pos && entries[pos - 1].score > score; --pos) entries[pos] = entries[pos - 1];
        entries[pos].move = move;
        entries[pos].score = score;
    }

    board getNext() {
        if (size)
            return entries[--size].move;
        else
            return 0;
    }

    MoveSorter() : size{ 0 } { }
};