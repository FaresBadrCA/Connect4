#include "Solver.hpp"

namespace fs = std::filesystem;

int main() {
	Solver S;


	//Position P("1614526522231152336277346531355667177");
	//board w = P.win_in_3();
	//S.display_board(w);
	//int eval = S.negamax(P, -43, 43);
	//eval = S.ply_score_to_move_score(eval);
	//std::cout << eval << "\n";
	
	fs::path test_folder = "Tests";
	std::string test_filename = "Test_L2_R2";
	
	// Prepare output file for results of solution
	std::time_t t = std::time(nullptr);
	char dt_str[100];
	std::strftime(dt_str, sizeof(dt_str), "%y%m%d-%H%M%S", std::localtime(&t));
	
	std::ofstream strm( std::string(dt_str) + "_" + test_filename + ".csv");
	std::string test_file_str = test_folder.append(test_filename).string();
	S.test_file(test_file_str, std::cout);
	}