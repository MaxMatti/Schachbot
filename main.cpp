#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include "board.hpp"
#include "move.hpp"

std::string read_board() {
	std::string result;
	std::string tmp;
	do {
		std::getline(std::cin, tmp);
		if (tmp.empty() && result.empty()) {
			break;
		}
		while (tmp.length() < 8) {
			tmp += " ";
		}
		result += tmp;
	} while (result.length() < 64);
	return result.substr(0, 64);
}

std::string read_move() {
	std::string result;
	std::cin >> result;
	return result;
}

void signal_handler(int signal [[maybe_unused]]) {
	exit(0);
}

template<bool amIWhite>
Move getMove(Board<amIWhite> current_situation) {
	std::string tmp;
	while (tmp.length() != 4 && tmp.length() != 5) {
		std::cin >> tmp;
	}
	std::uint64_t from = 1ul << (63 - (7 - (tmp[0] - (tmp[0] < 'I' ? 'A' : 'a')) + (tmp[1] - '1') * 8));
	std::uint64_t to = 1ul << (63 - (7 - (tmp[2] - (tmp[2] < 'I' ? 'A' : 'a')) + (tmp[3] - '1') * 8));
	piece turn_to = tmp.length() > 4 ? getPiece(tmp[4]) : None;
	return Move{from, to, current_situation.at(from), turn_to};
}

int main(int argc, char const *argv[]) {
	std::signal(SIGINT, signal_handler);
	if (argc < 1) {
		std::cout << *argv << "\n";
	}
	std::array<int, 128> initValues = {0};
	initValues[None] = 0;
	initValues[OwnKing] = 1000;
	initValues[OwnQueen] = 9;
	initValues[OwnRook] = 5;
	initValues[OwnBishop] = 3;
	initValues[OwnKnight] = 3;
	initValues[OwnPawn] = 1;
	initValues[EnemyKing] = -1000;
	initValues[EnemyQueen] = -9;
	initValues[EnemyRook] = -5;
	initValues[EnemyBishop] = -3;
	initValues[EnemyKnight] = -3;
	initValues[EnemyPawn] = -1;
	/*std::string initBoard = "R       "
							"        "
							"        "
							"  k     "
							"        "
							"     K  "
							"        "
							"        ";*/
	std::string initBoard = "rnbqkbnr"
							"pppppppp"
							"        "
							"        "
							"        "
							"        "
							"PPPPPPPP"
							"RNBQKBNR";
	/*std::string initBoard = " k  n  r"
							" pp   p "
							"p  rN qp"
							"Q    p  "
							"   p    "
							"     P P"
							"PPP  PP "
							" KR R   ";*/
	Board<true> current_situation(initBoard);
	Board<false> other_situation;
	while (current_situation.isValid()) {
		std::cout << current_situation;
		Move move;
		move = getMove(current_situation);
		if (move == Move() || !current_situation.isValidMove(move)) {
			break;
		}
		other_situation = current_situation.applyMove(move);
		std::cout << other_situation;
		std::vector<Move> validMoves;
		other_situation.forEachValidMove([&](auto i) {
			if (!other_situation.isValidMove(i)) {
				std::cout << "invalid ";
			}
			std::cout << "move: " << i << "(" << other_situation.at(i.move_from) << "->" << other_situation.at(i.move_to) << ")" << std::endl;
			validMoves.push_back(i);
		});
		auto chosenMove = validMoves.at(std::rand() % validMoves.size());
		std::cout << "chose: " << chosenMove << std::endl;
		current_situation = other_situation.applyMove(chosenMove);
	}
	current_situation.forEachValidMove([&](auto i) {
		auto tmp = current_situation.applyMove(i);
		std::cout << i << (tmp.isValid() ? "   (valid)\n" : " (invalid)\n");
	});
	std::cout << current_situation;
	return 0;
}
