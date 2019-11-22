#pragma once

#include "bot.hpp"
#include <cstddef>
#include <fstream>
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
    std::map<Bot, std::map<Board<true>, Move>> whiteMoveCache;
    std::map<Bot, std::map<Board<false>, Move>> blackMoveCache;

    void playGame(
        std::vector<std::pair<Bot, int>>::iterator bot1,
        std::vector<std::pair<Bot, int>>::iterator bot2,
        std::list<outcome>::reverse_iterator result);

public:
    Tournament() {}
    Tournament(std::vector<std::pair<Bot, int>> contestants)
        : contestants(contestants) {}
    Tournament(const Tournament& previous, const float& mutationIntensity, std::mt19937& generator);
    Tournament(std::ifstream in) { loadTournament(in); }
    bool addContestant(const Bot& newContestant);
    bool addContestant(Bot&& newContestant);
    size_t size() const { return contestants.size(); }
    void evaluate(const bool loud);
    void prepareNextRound(
        const float& mutationIntensity,
        std::mt19937& generator,
        const std::size_t winners,
        const std::size_t generationSize);
    std::string extraInfo() const;

    void saveTournament(std::ofstream& out) const;
    void loadTournament(std::ifstream& in);

    friend std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
};

std::ostream& operator<<(std::ostream& stream, const outcome& result);
std::ostream& operator<<(std::ostream& stream, const Tournament& tournament);
