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
	//Bot::finalizeTimings();
	std::cout << "Analyzed " << Bot::timeCounter << " situations in " << (double) Bot::timeSum / CLOCKS_PER_SEC << " seconds.\n";
	//std::cout << "Offline Algorithm:\n";
	std::cout << "That's " << Bot::timeCounter * CLOCKS_PER_SEC / (double) Bot::timeSum << " situations per second or " << (double) Bot::timeSum * 1000000 / Bot::timeCounter / CLOCKS_PER_SEC << " µs per situation.\n";
	std::cout << "Possible moves: " << Bot::maxPossibleMoves << "\n";
	std::cout << "Valid moves: " << Bot::maxValidMoves << "\n";
	/*std::cout << "Online Algorithm:\n";
	std::cout << "That's " << CLOCKS_PER_SEC / Bot::timeMean << " situations per second of " << Bot::timeMean * 1000000 / CLOCKS_PER_SEC << "µs per situation with a standard derivation of " << std::sqrt(Bot::timeVariance) * 1000000 / CLOCKS_PER_SEC << "µs.\n";*/
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
	std::string initBoard = "rbnqknbr"
							"pppppppp"
							"        "
							"        "
							"        "
							"        "
							"PPPPPPPP"
							"RBNQKNBR";
	Board current_situation(initBoard);
	bool invert = false;
	while (current_situation.isValid()) {
		Move move;
		for (int i = 0; i < 20; ++i) {
			std::cout << current_situation;
			std::cout << std::right << std::setw(4) << i << ": ";
			std::tie(move, std::ignore, i) = initBot.getQuickMove(current_situation, i, false);
			Board tmp_situation = current_situation;
			invert = false;
			for (int j = i; j > 0; --j) {
				if (invert) {
					std::cout << move.invert() << " -> ";
					//std::cout << "\n" << Board(tmp_situation, true);
				} else {
					std::cout << move << " -> ";
					//std::cout << "\n" << tmp_situation;
				}
				tmp_situation = tmp_situation.applyMove(move, true);
				move = std::get<0>(initBot.getQuickMove(tmp_situation, j, false));
				invert = !invert;
			}
			std::cout << move << "\n";
			std::cout << "Analyzed " << Bot::timeCounter << " situations in " << (double) Bot::timeSum / CLOCKS_PER_SEC << " seconds.\n";
			std::cout << "That's " << Bot::timeCounter * CLOCKS_PER_SEC / (double) Bot::timeSum << " situations per second or " << (double) Bot::timeSum * 1000000 / Bot::timeCounter / CLOCKS_PER_SEC << " µs per situation.\n";
			std::flush(std::cout);
		}
		move = std::get<0>(initBot.getQuickMove(current_situation, 20, false));
		current_situation = current_situation.applyMove(move, true);
		std::cout << current_situation.display();
		if (move == Move()) {
			break;
		}
	}
	/*
	std::random_device dev;
	std::mt19937 gen(dev());
	Tournament first;
	first.addContestant(initBot);
	for (auto i = 1; i < 4; ++i) {
		if (!first.addContestant({initBot, 0.1, gen})) {
			--i;
		}
	}
	//std::cout << first;
	for (auto i = 0; i < 10; ++i) {
		first.evaluate(false);
		//std::cout << first;
		first = Tournament(first, 0.1, gen);
	}
	//Bot::finalizeTimings();*/
	std::cout << "Analyzed " << Bot::timeCounter << " situations in " << (double) Bot::timeSum / CLOCKS_PER_SEC << " seconds.\n";
	//std::cout << "Offline Algorithm:\n";
	std::cout << "That's " << Bot::timeCounter * CLOCKS_PER_SEC / (double) Bot::timeSum << " situations per second or " << (double) Bot::timeSum * 1000000 / Bot::timeCounter / CLOCKS_PER_SEC << " µs per situation.\n";
	std::cout << "Possible moves: " << Bot::maxPossibleMoves << "\n";
	std::cout << "Valid moves: " << Bot::maxValidMoves << "\n";
	/*std::cout << "Online Algorithm:\n";
	std::cout << "That's " << CLOCKS_PER_SEC / Bot::timeMean << " situations per second of " << Bot::timeMean * 1000000 / CLOCKS_PER_SEC << "µs per situation with a standard derivation of " << std::sqrt(Bot::timeVariance) * 1000000 / CLOCKS_PER_SEC << "µs.\n";*/
	return 0;
}
