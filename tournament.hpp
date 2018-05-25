#include <vector>
#include "bot.hpp"

#pragma once

class Tournament {
private:
	std::vector<std::tuple<Bot, int>> contestants;

	void playGame(std::vector<std::tuple<Bot, int>>::iterator bot1, std::vector<std::tuple<Bot, int>>::iterator bot2, const bool rated, const bool loud);

public:
	Tournament() {};
	Tournament(std::vector<std::tuple<Bot, int>> contestants) : contestants(contestants) {};
	Tournament(const Tournament& previous, const float& mutationIntensity, std::mt19937& generator);
	bool addContestant(const Bot& newContestant);
	void evaluate(const bool loud);

	friend std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
};

std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
