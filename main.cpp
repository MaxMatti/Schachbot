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
	Board current_situation(initBoard);
	while (current_situation.isValid()) {
		std::cout << current_situation.display();
		Move move;
		for (unsigned int i = 0; i < 21; ++i) {
			// std::cout << current_situation;
			std::cout << std::right << std::setw(4) << i << ": ";
			std::clock_t starttime = std::clock();
			unsigned int tmp = 0;
			std::tie(move, std::ignore, tmp) = initBot.getQuickMove(current_situation, i, false);
			if (tmp < i) {
				std::cout << tmp << "<" << i << "\n";
				exit(0);
			}
			i = tmp;
			unsigned long long int time = std::clock() - starttime;
			std::cout << "(" << (double) time / CLOCKS_PER_SEC << "s, max: " << Bot::maximumPossibleMoves << "/" << Bot::maximumValidMoves << ") ";
			std::cout << move << "\n";
			std::flush(std::cout);
			if ((double) time / CLOCKS_PER_SEC > 0.5) {
				break;
			}
		}
		if (move == Move()) {
			break;
		}
		current_situation = current_situation.applyMove(move, true);
	}
	auto validateMove = [&current_situation](const Move& move){
		Board new_situation = current_situation.applyMove(move, false);
		auto tmp = new_situation.getNextPiece(OwnKing, 0);
		return tmp < 64 && !new_situation.isThreatened(tmp);
	};
	for (auto i : current_situation.getValidMoves()) {
		std::cout << i << (validateMove(i) ? "   (valid)\n" : " (invalid)\n");
	}
	std::cout << current_situation.print();
	return 0;
}
