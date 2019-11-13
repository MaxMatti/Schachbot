#pragma once

#include "bot.hpp"
#include <list>
#include <vector>

enum outcome {
    notPlayed = 0,
    whiteWon = 1,
    blackWon = 2,
    draw = 3,
    lineBreak = 4,
};

class Tournament {
private:
    std::vector<std::pair<Bot, int>> contestants;

    void playGame(
        std::vector<std::pair<Bot, int>>::iterator bot1,
        std::vector<std::pair<Bot, int>>::iterator bot2,
        std::list<outcome>::reverse_iterator result);

public:
    Tournament() {}
    Tournament(std::vector<std::pair<Bot, int>> contestants)
        : contestants(contestants) {}
    Tournament(const Tournament& previous, const float& mutationIntensity, std::mt19937& generator);
    bool addContestant(Bot&& newContestant);
    void evaluate(const bool loud);

    friend std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
};

std::ostream& operator<<(std::ostream& stream, const outcome& result);
std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
