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
	std::array<int, 15> values;

public:
	static std::atomic<std::size_t> maximumPossibleMoves;
	static std::atomic<std::size_t> maximumValidMoves;
	Bot();
	Bot(std::array<int, 15> new_values) : values(new_values) {};
	Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator);

	friend std::ostream& operator<<(std::ostream& stream, const Bot& bot);
	friend bool operator<(const Bot& bot1, const Bot& bot2);
	friend bool operator==(const Bot& bot1, const Bot& bot2);
};

template<typename T>
void updateMaximum(std::atomic<T>& maximum_value, T const& value) noexcept;
std::ostream& operator<<(std::ostream& stream, const Bot& bot);
bool operator<(const Bot& bot1, const Bot& bot2);
bool operator==(const Bot& bot1, const Bot& bot2);
