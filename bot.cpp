#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include "bot.hpp"
#include "move.hpp"

std::atomic<unsigned long long int> Bot::timesum = 1;
std::atomic<unsigned int> Bot::timecounter = 1;

Bot::Bot() {
	this->values[OwnKing] = 127;
	this->values[OwnQueen] = 9;
	this->values[OwnRook] = 5;
	this->values[OwnBishop] = 3;
	this->values[OwnKnight] = 3;
	this->values[OwnPawn] = 1;
	this->values[EnemyKing] = -127;
	this->values[EnemyQueen] = -9;
	this->values[EnemyRook] = -5;
	this->values[EnemyBishop] = -3;
	this->values[EnemyKnight] = -3;
	this->values[EnemyPawn] = -1;
}
/*
Bot::Bot(const Board& start_situation) {
	this->getMove(start_situation);
}
*/
Bot::Bot(const Bot& previous) : /*assessments(previous.assessments), */values(previous.values) {}

Bot::Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator) : /*assessments(previous.assessments), */values(previous.values) {
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	for (int& i : this->values) {
		if (distribution(generator) < mutationIntensity) {
			i += 1;
		} else if (distribution(generator) < mutationIntensity) {
			i -= 1;
		}
	}
}

std::tuple<Move, int> Bot::getQuickMove(const Board& situation, const unsigned int depth, const bool loud) {
	auto addFigureWeight = [this](int before, const piece& figure) {
		return before + this->values[figure];
	};
	auto getAssessment = [addFigureWeight](const Board& situation) {
		return std::accumulate(situation.fields.begin(), situation.fields.end(), 0, addFigureWeight);
	};
	if (depth == 0) {
		std::clock_t c_start = std::clock();
		std::vector<Move> possibleMoves = situation.getValidMoves();
		if (loud) {
			std::cout << "Valid Moves:";
			std::for_each(possibleMoves.begin(), possibleMoves.end(), [](Move& move){ std::cout << " " << move; });
			std::cout << "\n";
		}
		if (possibleMoves.empty()) {
			return std::make_tuple(Move(), std::numeric_limits<int>::min() + 1);
		}
		Move result = *std::max_element(possibleMoves.begin(), possibleMoves.end(), [&](const Move& a, const Move& b) {
			// comp function should return true if a<b but it's reversed because actually I want (-a)<(-b) which is a>b
			auto tmp = getAssessment(situation.applyMove(a, true)) > getAssessment(situation.applyMove(b, true));
			return tmp;
		});
		Bot::timesum += std::clock() - c_start;
		++Bot::timecounter;
		return std::make_tuple(result, -getAssessment(situation.applyMove(result, true)));
	} else {
		std::vector<Move> possibleMoves = situation.getValidMoves();
		Move bestMove = *(possibleMoves.begin());
		int bestScore = std::numeric_limits<int>::min() + 1;
		for (auto move : possibleMoves) {
			auto tmp = this->getQuickMove(situation.applyMove(move, true), depth - 1);
			if (-std::get<1>(tmp) > bestScore) {
				bestScore = -std::get<1>(tmp);
				bestMove = move;
			}
		}
		return std::make_tuple(bestMove, bestScore);
	}
}

std::ostream& operator<<(std::ostream& stream, const Bot& bot) {
	stream << "Bot(";
	for (std::size_t i = 0; i < bot.values.size(); ++i) {
		if (getChessSymbol((piece) i) != "x") {
			stream << (piece) i << ": " << bot.values[i] << ", ";
		}
	}
	stream << ")";
	return stream;
}

bool operator<(const Bot& bot1, const Bot& bot2) {
	return bot1.values < bot2.values;
}

bool operator==(const Bot& bot1, const Bot& bot2) {
	return bot1.values == bot2.values;
}
