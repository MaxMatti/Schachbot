#pragma once

#include <atomic>
#include <ctime>
#include <mutex>
#include <random>
#include <unordered_map>
#include <tuple>
#include "board.hpp"
#include "move.hpp"

class Bot {
private:
	// Board -> Move, depth, strength
	std::array<int, 128> values;

public:
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
