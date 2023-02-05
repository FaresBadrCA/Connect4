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
    int alpha = -(Position::HEIGHT * Position::WIDTH) - 1; // -INF
    int beta = Position::HEIGHT * Position::WIDTH + 1; // +INF

    max_depth = 1;
    depth_limited = true;
    int depth = 0;
    Window w(alpha, beta);
    while (depth_limited) {
        depth_limited = false;
        w = negamax(P, w.alpha, w.beta, depth);
        max_depth++; // iterative deepening
    }
    return w.beta;
}

// Evaluation interpretation: +x means a win can be forced in x plies
// -x means the opponent can force a win in x plies
// 0 means neither player can force a win
Window Solver::negamax(Position &P, int alpha, int beta, int depth) {
    if (P.gameover_zero()) {
        int score = -(Position::BOARD_SIZE - P.nb_moves + 1);
        return Window(score, score);
    }

    if (P.nb_moves == 42) { 
        return Window(0, 0); 
    }

    int min = -(Position::BOARD_SIZE - P.nb_moves - 1); // Worst possible score if opponent forces a win in 2 plies
    int max = (Position::BOARD_SIZE - P.nb_moves) ; // Best possible score if the next move forces a win

    // there is an [alpha beta] window based on prio information
    // there is a [min, max] window based on how deep we are in the game tree
    // combine those two into the smallest possible window
    if (min > alpha) {
        alpha = min;
        if (alpha >= beta) return Window(alpha, alpha);
    }

    if (max < beta) {
        beta = max;
        if (alpha >= beta) return Window(alpha, alpha);
    }
    
    board key = P.key();
    T.get(key, alpha, beta);
    if (alpha >= beta) {
        return Window(alpha, alpha);
    }

   if (depth >= max_depth) {  
        depth_limited = true;
        // we didn't explore this node, but we still got some information. the game is not over yet
        // the best score our parent can reach is limited here. 
        // max = P.SCORE(); 
        // min = ( (Position::WIDTH * Position::HEIGHT) - #moves )
        return Window(alpha, beta); 
   }

    board legal = P.get_legal();

    // There are min and max scores related to how many moves into the game we are.
    // we can use those to further tighten our alpha beta window

    
    int min_alpha_i = Position::BOARD_SIZE; // Track lowest alpha for any child. used to update parent's Beta.
    for (int i = 0; i < Position::WIDTH; ++i){
        Position next_p(P);
        board move = legal & next_p.COL_MASK(Position::MOVE_ARRAY()[i]);
        if (move) {
            next_p.play_move(move);
            Window window_i = negamax(next_p, -beta, -alpha, depth + 1);
            T.put(next_p.key(), window_i.alpha, window_i.beta);

            // parent's score is at least alpha. Equal to -1 * lowest child beta
            if (-1 * window_i.beta > alpha) {
                alpha = -1 * window_i.beta;
            }

            if (window_i.alpha < min_alpha_i) {
                min_alpha_i = window_i.alpha;
            }

            if (alpha >= beta) {
                break; // no need to keep iterating through the children
            }
        }
    }

    // parent's score is at most Beta. If the lowest child score is alpha_i, then parent can at most get -1 * alpha_i
    if (-1 * min_alpha_i < beta) {
        beta = -1 * min_alpha_i;
    }
    return Window(alpha, beta);
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

        int ply_score = alpha_beta(p);

        // divide ply_score by 2 and round away from zero
        int move_score = ply_score_to_move_score(ply_score);

        assert(move_score == eval);
        auto n_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(clock.now() - t1).count();
        strm << moves << ", " << n_microseconds << ", " << Position::n_positions_evaluated << "\n";
    }
}