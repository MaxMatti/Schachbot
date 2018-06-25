#include <atomic>
#include <ctime>
#include <mutex>
#include <random>
#include <unordered_map>
#include <tuple>
#include "board.hpp"
#include "move.hpp"

#pragma once

class Bot {
private:
	// Board -> Move, depth, strength
	std::array<int, 128> values;

public:
	static std::atomic<unsigned long long int> timeSum;
	static std::atomic<unsigned long int> timeCounter;
	static std::atomic<unsigned long int> maxPossibleMoves;
	static std::atomic<unsigned long int> maxValidMoves;
	static double timeMean;
	static double timeM2;
	static double timeVariance;
	static std::mutex timeMutex;

	static void updateTimings(const unsigned long long int time, const unsigned long int possibleMoveSize, const unsigned long int validMoveSize);
	static void finalizeTimings();

	Bot();
	Bot(const Bot& previous);
	Bot(std::array<int, 128> values) : values(values) {};
	Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator);
	std::tuple<Move, int, unsigned int> getQuickMove(const Board& situation, const unsigned int depth, const bool loud = false);

	friend std::ostream& operator<<(std::ostream& stream, const Bot& bot);
	friend bool operator<(const Bot& bot1, const Bot& bot2);
	friend bool operator==(const Bot& bot1, const Bot& bot2);
};

template<typename T>
void updateMaximum(std::atomic<T>& maximum_value, T const& value) noexcept;
std::ostream& operator<<(std::ostream& stream, const Bot& bot);
bool operator<(const Bot& bot1, const Bot& bot2);
bool operator==(const Bot& bot1, const Bot& bot2);
