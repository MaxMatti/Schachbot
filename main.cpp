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

int main(int argc, char const *argv[]) {
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
	std::random_device dev;
	std::mt19937 gen(dev());
	Tournament first;
	std::cout << first;
	first.addContestant(initBot);
	std::cout << first;
	for (auto i = 1; i < 4; ++i) {
		if (!first.addContestant({initBot, 0.1, gen})) {
			--i;
		}
	}
	std::cout << first;
	for (auto i = 0; i < 1; ++i) {
		first.evaluate(true);
		std::cout << first;
		first = Tournament(first, 0.1, gen);
	}
	return 0;
}
