#include "Solver.hpp"

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

int32_t Solver::alpha_beta(Position& P) {

    // if P.gameoverzero: return score
    // if P.winning_moves: return score

    int min = -(Position::HEIGHT * Position::WIDTH) - 1; // -INF
    int max = Position::HEIGHT * Position::WIDTH + 1; // +INF
    int med = 0;
    while (max > min) {
        // For a window [min, max], test if score > med, where med is the largest absolute value of: (min+max)/2 or (min/2) or (max/2)
        med = min + (max -min) / 2; // Note: (min+max)/2 gets trapped in [-1, 0] window, but min + (max-min)/2 does not
        if (med >= 0 && med < max / 2) med = max / 2;
        else if (med <= 0 && med > min / 2) med = min / 2;

        int score = negamax(P, med, med + 1); // Check if score is > med or <= med
        if (score > med) min = score;
        else max = score;
    }
    return min;
}  

// Evaluation interpretation: +x means a win can be forced in x plies
// -x means the opponent can force a win in x plies
// 0 means neither player can force a win
int Solver::negamax(Position &P, int alpha, int beta) {
    
    if (P.nb_moves == 42) {
        return 0;
    }

    board nonlosing_moves = P.nonlosing_moves();
    if (!nonlosing_moves) {
        return -(Position::BOARD_SIZE - P.nb_moves - 1);
    }

    board key = P.key();
    int lower_bound = -(Position::BOARD_SIZE - P.nb_moves - 1);
    int upper_bound = (Position::BOARD_SIZE - P.nb_moves);
    T.get(key, lower_bound, upper_bound);

    if (lower_bound > alpha) {
        alpha = lower_bound;
        if (alpha >= beta) return alpha;
    }
    if (upper_bound < beta) {
        beta = upper_bound;
        if (alpha >= beta) return beta;
    }

    // There are min and max scores related to how many moves into the game we are.
    // we can use those to further tighten our alpha beta window    
    int min_alpha_i = Position::BOARD_SIZE; // Track lowest alpha for any child. used to update parent's Beta.
    for (int i = 0; i < Position::WIDTH; ++i){
        Position next_p(P);
        board move = nonlosing_moves & next_p.COL_MASK(Position::MOVE_ARRAY()[i]);
        if (move) {
            next_p.play_move(move);
            int score_i = negamax(next_p, -beta, -alpha);
            // T.put(next_p.key(), window_i.alpha, window_i.beta);

            // parent's score is at least alpha. Equal to -1 * lowest child beta
            if (-1 * score_i > alpha) {
                alpha = -1 * score_i;
            }

            if (alpha >= beta) {
                T.put(key, alpha, upper_bound); // Score >= alpha
                return alpha; // no need to keep iterating through the children
            }
        }
    }
    T.put(key, lower_bound, alpha); // Score <= alpha
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
    while (std::getline(f, line)) {
        auto t1 = clock.now();
        std::size_t space_pos = line.find(" ");
        std::string moves = line.substr(0, line.find(" "));
        int eval = std::stoi(line.substr(space_pos + 1, line.length()));

        Position p(moves);
        
        //p.display();
        //std::cout << line << "\n";
        
        int ply_score = alpha_beta(p);

        // divide ply_score by 2 and round away from zero
        auto n_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(clock.now() - t1).count();
        strm << moves << ", " << n_microseconds << ", " << Position::n_positions_evaluated << "\n";

        int move_score = ply_score_to_move_score(ply_score);
        assert(move_score == eval);
    }
}