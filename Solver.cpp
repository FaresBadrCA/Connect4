/*
ALPHA-BETA EXPLAINED:

We define alpha as the highest score that the current player can force for a position, given what we've explored.
We define beta as the lowest score that the other player can force the current player into before getting to the current positoin, given what we've explored.

Thus, we have a window [alpha, beta]: 
If alpha gets greater than beta, then optimal play for the other player means we never get to this position, so there is no point continuing evaluating it.
If alpha equals beta, there is also no point evaluating this position since the opponent knows they can force a score of beta in another position, and the current position has score beta or higher.

WITH NEGAMAX: 
We are given a parent position with alpha-beta window [a,b]. When we evaluate its child position, it will have window [-b, -a]
That is because the parent can choose a move to have a score of 'a', forcing the child player to have a score of '-a'. Thus, beta = -a
Additionally, the child player can force the parent into a position of value 'b', which means the child player can have a value as high as -b through previous choices. Thus, alpha = -b
 

PLY SCORES:
0 means with perfect-play the game is a draw from this position
+n means current player can force a win 'n' plies before the full board state.
-n means next player can force a win 'n' plies before the full board

MOVE SCORES: are +/- n when there are n/2 (rounded up) spots remaining on the board before the last move
MOVE_SCORE = (PLYSCORE / 2) round away from zero
*/

#include "Solver.hpp"
#include "MoveSorter.hpp"

// Constructor
Solver::Solver() : nodeCount{ 0 } {
    for (int i = 0; i < Position::WIDTH; i++)
        columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
    // initialize the column exploration order, starting with center columns
    // example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
}

int32_t Solver::alpha_beta(Position& P) {

    int min = -(Position::HEIGHT * Position::WIDTH) - 1; // -INF
    int max = Position::HEIGHT * Position::WIDTH + 1; // +INF
    while (min < max) {                    // iteratively narrow the min-max exploration window
        int med = min + (max - min) / 2;
        if (med <= 0 && min / 2 < med) med = min / 2;
        else if (med >= 0 && max / 2 > med) med = max / 2;
        int r = negamax(P, med, med + 1);   // use a null depth window to know if the actual score is greater or smaller than med
        if (r <= med) max = r;
        else min = r;
    }
    return min;
}  

// Evaluation interpretation: +x means a win can be forced in x plies
// -x means the opponent can force a win in x plies
// 0 means neither player can force a win
int Solver::negamax(Position &P, int alpha, int beta) {
    
    board possible = P.nonlosing_moves();

    if (!possible) {
        return -(Position::BOARD_SIZE - P.nb_moves) / 2; // -(Position::BOARD_SIZE - P.nb_moves - 1);
    }

    if (P.nb_moves >= Position::BOARD_SIZE - 2) {
        return 0;
    }

    // board nonlosing_moves = P.nonlosing_moves();
    //if (P.win_in_3() & nonlosing) {
    //    return (Position::BOARD_SIZE - P.nb_moves - 2);
    //}

    int lower_bound = -(Position::BOARD_SIZE - 2 - P.nb_moves) / 2;  // we're not losing this round so lower bound is losing in 4 plies
    if (lower_bound > alpha) {
        alpha = lower_bound;
        if (alpha >= beta) return alpha;
    }

    int upper_bound = (Position::BOARD_SIZE - 1 - P.nb_moves) / 2; // we're not winning this round, so upper bound is winning in 3 plies
    if (upper_bound < beta) {
        beta = upper_bound;
        if (alpha >= beta ) return beta ;
    }

    board key = P.key();
    int val = T.get(key);
    if (val) {
        if (val < 0) { // we have an lower bound
            lower_bound = val + Position::MAX_SCORE + 1;
            if (alpha < lower_bound) {
                alpha = lower_bound;                     // there is no need to keep beta above our max possible score.
                if (alpha >= beta) return alpha;  // prune the exploration if the [alpha;beta] window is empty.
            }
        }
        else { // we have an upper bound
            upper_bound = val - Position::MAX_SCORE - 1;
            if (beta > upper_bound) {
                beta = upper_bound;                     // there is no need to keep beta above our max possible score.
                if (alpha >= beta) return beta;  // prune the exploration if the [alpha;beta] window is empty.
            }
        }
    }

    MoveSorter moves;
    for (int i = Position::WIDTH; i--; )
        if (uint64_t move = possible & Position::COL_MASK(columnOrder[i]))
            moves.add(move, P.moveScore(move));

    while (board next = moves.getNext()) {
        Position next_p(P);
        next_p.play_move(next);
        int score = -negamax(next_p, -beta, -alpha);

        if (score >= beta) {
            T.put(key, score - Position::MAX_SCORE - 1); // save the lower bound of the position
            return score; // no need to keep iterating through the children
        }

        if (score > alpha) {
            alpha = score;
        }
    }
    T.put(key, alpha + Position::MAX_SCORE + 1); // save the upper bound of the position
    return alpha;
}

// Writes positions, time for evaluation and #positions evaluated into output stream
void Solver::test_file(std::string filename, std::ostream &strm) {
    std::ifstream f;
    std::string line;
    f.open(filename, std::fstream::in);
    if (!f.is_open()) {
        std::cout << "Failed to open file: " << filename << "\n";
        return;
    }

    auto clock = std::chrono::steady_clock();
    long long total_microseconds = 0;
    while (std::getline(f, line)) {
        auto t1 = clock.now();
        std::size_t space_pos = line.find(" ");
        std::string moves = line.substr(0, line.find(" "));
        int eval = std::stoi(line.substr(space_pos + 1, line.length()));

        Position p(moves);        
        int move_score = alpha_beta(p);

        // divide ply_score by 2 and round away from zero
        auto n_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(clock.now() - t1).count();
        total_microseconds += n_microseconds;
        strm << moves << ", " << n_microseconds << ", " << Position::n_positions_evaluated << "\n";

        // move_score = ply_score_to_move_score(move_score);
        assert(move_score == eval);
    }
    std::cout << "Total #Seconds: " << total_microseconds / 1e6 << "\n";
    std::cout << "Total #Positions: " << Position::n_positions_evaluated << "\n";
}