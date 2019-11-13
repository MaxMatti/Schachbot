#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include "tournament.hpp"
#include <iostream>
#include <random>

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    Bot parent;
    std::mt19937 engine;
    Tournament tournament;
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.evaluate(true);
    std::cout << tournament;
    tournament.prepareNextRound(0.1f, engine, 4, 8);
    tournament.evaluate(true);
    std::cout << tournament;
    return 0;
}
