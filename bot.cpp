#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include "bot.hpp"
#include "move.hpp"

std::atomic<unsigned long long int> Bot::timeSum = 1;
std::atomic<unsigned int> Bot::timeCounter = 1;
double Bot::timeMean = 1;
double Bot::timeM2 = 1;
double Bot::timeVariance = 1;
std::mutex Bot::timeMutex;

void Bot::updateTimings(const unsigned long long int time) {
	Bot::timeSum += time;
	++Bot::timeCounter;
	/*Bot::timeMutex.lock();
	double delta = time - Bot::timeMean;
	Bot::timeMean += delta / Bot::timeCounter;
	double delta2 = time - Bot::timeMean;
	Bot::timeM2 += delta * delta2;
	Bot::timeMutex.unlock();*/
}

void Bot::finalizeTimings() {
	Bot::timeVariance = Bot::timeM2 / Bot::timeCounter;
}

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

Bot::Bot(const Bot& previous) : values(previous.values) {}

Bot::Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator) : values(previous.values) {
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	for (int& i : this->values) {
		if (distribution(generator) < mutationIntensity) {
			i += 1;
		} else if (distribution(generator) < mutationIntensity) {
			i -= 1;
		}
	}
}

std::tuple<Move, int, unsigned int> Bot::getQuickMove(const Board& situation, const unsigned int depth, const bool loud) {
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
			std::cout << "Possible Moves:";
			std::for_each(possibleMoves.begin(), possibleMoves.end(), [](Move& move){ std::cout << " " << move; });
			std::cout << "\n";
		}
		auto validateMove = [&situation = std::as_const(situation)](const Move& move){
			Board new_situation = situation.applyMove(move, false);
			auto tmp = new_situation.getNextPiece(OwnKing, 0);
			return tmp < 64 && !new_situation.isCheck();
		};
		std::vector<std::tuple<Move, int, unsigned int>> validMoves;
		for (auto i : possibleMoves) {
			if (validateMove(i)) {
				validMoves.push_back(std::make_tuple(i, -getAssessment(situation.applyMove(i)), 0));
			}
		}
		//std::copy_if(possibleMoves.begin(), possibleMoves.end(), std::back_inserter(validMoves), validateMove);
		if (loud) {
			std::cout << "Valid Moves:";
			std::for_each(validMoves.begin(), validMoves.end(), [](auto& move){ std::cout << " " << std::get<0>(move); });
			std::cout << "\n";
		}
		if (validMoves.empty()) {
			return std::make_tuple(Move(), std::numeric_limits<int>::min() + 1, std::numeric_limits<unsigned int>::max());
		}
		/*std::vector<int> assessments;
		std::transform(validMoves.begin(), validMoves.end(), std::back_inserter(assessments), [&getAssessment, &situation](const Move& move){return -getAssessment(situation.applyMove(move, true)); });
		std::size_t pos = std::distance(assessments.begin(), std::max_element(assessments.begin(), assessments.end()));*/
		Bot::updateTimings(std::clock() - c_start);
		return *std::max_element(validMoves.begin(), validMoves.end(), [](auto a, auto b){return std::get<1>(a) < std::get<1>(b);});
		//return std::make_tuple(validMoves[pos], assessments[pos]);
	} else {
		std::vector<Move> possibleMoves = situation.getValidMoves();
		//std::vector<Move> validMoves;
		auto validateMove = [&situation = std::as_const(situation)](const Move& move){
			Board new_situation = situation.applyMove(move, false);
			auto tmp = new_situation.getNextPiece(OwnKing, 0);
			return tmp < 64 && !new_situation.isThreatened(tmp);
		};
		std::vector<std::tuple<Move, int, unsigned int>> validMoves;
		for (auto i : possibleMoves) {
			if (validateMove(i)) {
				validMoves.push_back(std::make_tuple(i, -getAssessment(situation.applyMove(i)), 0));
			}
		}
		//std::copy_if(possibleMoves.begin(), possibleMoves.end(), std::back_inserter(validMoves), validateMove);
		if (validMoves.empty()) {
			return std::make_tuple(Move(), std::numeric_limits<int>::min() + 1, std::numeric_limits<unsigned int>::max());
		}
		if (depth > 3) {
			for (auto& it : validMoves) {
				std::tie(std::ignore, std::get<1>(it), std::get<2>(it)) = this->getQuickMove(situation.applyMove(std::get<0>(it), false), 0);
			}
			std::nth_element(validMoves.begin(), validMoves.begin() + std::min(validMoves.size() / 2, 20ul), validMoves.end(), [](const auto& a, const auto& b){ return std::get<1>(a) > std::get<1>(b); });
			validMoves.resize(std::min(validMoves.size() / 2, 20ul));
		}
		for (auto& it : validMoves) {
			std::tie(std::ignore, std::get<1>(it), std::get<2>(it)) = this->getQuickMove(situation.applyMove(std::get<0>(it), true), depth - 1);
			std::get<1>(it) *= -1;
			std::get<2>(it) = std::max(std::get<2>(it), depth);
		}
		return *std::max_element(validMoves.begin(), validMoves.end(), [](const auto& a, const auto& b){ return std::get<1>(a) < std::get<1>(b); });
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
