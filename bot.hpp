#include <atomic>
#include <ctime>
#include <random>
#include <unordered_map>
#include <tuple>
#include "board.hpp"
#include "move.hpp"

#pragma once

class Bot {
private:
	// Board -> Move, depth, strength
	//std::unordered_map<Board, std::tuple<Move, unsigned int, int>> assessments;
	//std::unordered_map<piece, int> values;
	std::array<int, 128> values;

public:
	static std::atomic<unsigned long long int> timesum;
	static std::atomic<unsigned int> timecounter;

	Bot();
	//Bot(const Board& start_situation);
	Bot(const Bot& previous);
	Bot(std::array<int, 128> values) : values(values) {};
	Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator);
	std::tuple<Move, int> getQuickMove(const Board& situation, const unsigned int depth, const bool loud = false);
	//Move getMove(const Board& situation, unsigned int depth);
	//Move getMove(const Board& situation);
	//int getAssessment(const Board& situation);

	friend std::ostream& operator<<(std::ostream& stream, const Bot& bot);
	friend bool operator<(const Bot& bot1, const Bot& bot2);
	friend bool operator==(const Bot& bot1, const Bot& bot2);
};

std::ostream& operator<<(std::ostream& stream, const Bot& bot);
bool operator<(const Bot& bot1, const Bot& bot2);
bool operator==(const Bot& bot1, const Bot& bot2);
