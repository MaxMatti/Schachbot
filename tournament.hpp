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

    template <bool rated>
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

template <bool rated>
void Tournament::playGame(
    std::vector<std::pair<Bot, int>>::iterator bot1,
    std::vector<std::pair<Bot, int>>::iterator bot2,
    std::list<outcome>::reverse_iterator result) {
    std::string initBoard =
        "rnbqkbnr"
        "pppppppp"
        "        "
        "        "
        "        "
        "        "
        "PPPPPPPP"
        "RNBQKBNR";
    Board<true> currentSituation(initBoard);
    Board<false> reverseSituation;
    Move whiteMove;
    Move blackMove;

    while (true) {
        if (currentSituation.getFirstValidMove() == Move{}) {
            if constexpr (rated) {
                bot2->second += 3;
            }
            *result = draw;
            return;
        }
        whiteMove = bot1->first.getMove<6, false>(currentSituation);
        reverseSituation = currentSituation.applyMove(whiteMove);
        if (!reverseSituation.isValid()) {
            if constexpr (rated) {
                bot1->second += 1;
                bot2->second += 1;
            }
            *result = blackWon;
            return;
        }
        if (reverseSituation.getFirstValidMove() == Move{}) {
            if constexpr (rated) {
                bot1->second += 1;
                bot2->second += 1;
            }
            *result = draw;
            return;
        }
        blackMove = bot2->first.getMove<6, false>(reverseSituation);
        currentSituation = reverseSituation.applyMove(blackMove);
        if (!currentSituation.isValid()) {
            if constexpr (rated) {
                bot1->second += 3;
            }
            *result = whiteWon;
            return;
        }
    }
    __builtin_unreachable();
}
