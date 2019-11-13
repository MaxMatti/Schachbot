#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include "tournament.hpp"
#include <chrono>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

[[noreturn]] void signal_handler(int signal [[maybe_unused]]);

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGABRT, signal_handler);

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
    signal_handler(0);
    return 0;
}

[[noreturn]] void signal_handler(int signal [[maybe_unused]]) { exit(0); }
