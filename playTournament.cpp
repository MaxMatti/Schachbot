#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include "tournament.hpp"
#include <iostream>
#include <random>
#include <string>

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    Bot parent;
    float mutationIntensity = 0.4f;
    std::mt19937 engine;
    Tournament tournament;
    tournament.addContestant(parent);
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    tournament.addContestant(Bot(parent, mutationIntensity, engine));
    std::size_t tournamentLength = 100;
    if (argc > 1) {
        tournamentLength = std::stoll(argv[1], 0, 0);
    }
    for (std::size_t i = 0; i < tournamentLength; ++i) {
        tournament.evaluate(true);
        std::cout << tournament;
        tournament.prepareNextRound(mutationIntensity, engine, 4, 7);
        tournament.addContestant(parent);
        while (tournament.size() < 8) {
            tournament.addContestant(Bot(parent, mutationIntensity, engine));
        }
    }
    return 0;
}
