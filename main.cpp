#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include "tournament.hpp"

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

void signal_handler(int signal) {
	exit(0);
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
	Bot initBot(initValues);
	std::string initBoard = "R       "
							"        "
							"        "
							"  k     "
							"        "
							"     K  "
							"        "
							"        ";
	/*std::string initBoard = "rnbqkbnr"
							"pppppppp"
							"        "
							"        "
							"        "
							"        "
							"PPPPPPPP"
							"RNBQKBNR";*/
	/*std::string initBoard = " k  n  r"
							" pp   p "
							"p  rN qp"
							"Q    p  "
							"   p    "
							"     P P"
							"PPP  PP "
							" KR R   ";*/
	Board current_situation(initBoard);
	bool invert = false;
	while (current_situation.isValid()) {
		std::cout << current_situation.display();
		Move move;
		for (int i = 0; i < 9; i += 2) {
			// std::cout << current_situation;
			std::cout << std::right << std::setw(4) << i << ": ";
			std::clock_t starttime = std::clock();
			std::tie(move, std::ignore, i) = initBot.getQuickMove(current_situation, i, false);
			unsigned long long int time = std::clock() - starttime;
			std::cout << "(" << (double) time / CLOCKS_PER_SEC << "s) ";
			Board tmp_situation = current_situation;
			invert = false;
			for (int j = i; j > 0; --j) {
				if (invert) {
					std::cout << move.invert() << " -> ";
				} else {
					std::cout << move << " -> ";
				}
				// std::cout << "\n" << tmp_situation.applyMove(move, invert);
				tmp_situation = tmp_situation.applyMove(move, true);
				move = std::get<0>(initBot.getQuickMove(tmp_situation, j, false));
				invert = !invert;
				std::flush(std::cout);
			}
			std::cout << move << "\n";
			std::flush(std::cout);
		}
		move = std::get<0>(initBot.getQuickMove(current_situation, 20, false));
		current_situation = current_situation.applyMove(move, true);
		if (move == Move()) {
			break;
		}
	}
	std::cout << current_situation.display();
	return 0;
}
