#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include "tournament.hpp"
#include <fstream>
#include <iostream>
#include <random>
#include <string>

inline auto getSecondsSince(time_point start) {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
}

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    Bot parent;
    Tournament tournament;
    float mutationIntensity = 0.4f;
    std::mt19937 engine;
    std::size_t tournamentSize = 7;
    std::size_t tournamentLength = 100;
    std::string filename = "/tmp/chess.bin";
    if (argc > 1) {
        if (argc > 2) {
            if (argc > 3) {
                filename = argv[3];
            }
            tournamentLength = std::stoll(argv[2], 0, 0);
        }
        tournamentSize = std::stoll(argv[1], 0, 0);
    }
    std::ifstream savefile(filename.c_str(), std::ios_base::binary);
    if (savefile.good()) {
        tournament.loadTournament(savefile);
        std::cout << tournament;
        std::cout << tournament.extraInfo();
    }
    else {
        tournament.prepareNextRound(mutationIntensity, engine, 0, 0);
        tournament.addContestant(parent);
    }
    while (tournament.size() < tournamentSize) {
        tournament.addContestant(Bot(parent, mutationIntensity, engine));
    }
    tournament.prepareNextRound(mutationIntensity, engine, tournamentSize, tournamentSize);
    savefile.close();
    std::chrono::steady_clock::time_point startTime;
    for (std::size_t i = 0; i < tournamentLength; ++i) {
        startTime = std::chrono::steady_clock::now();
        tournament.evaluate(true);
        std::cout << "Tournament #" << (i + 1) << ": evaluated in " << getSecondsSince(startTime) << "s.\nSaving "
                  << tournament;
        std::ofstream savefile(filename.c_str(), std::ios_base::binary);
        tournament.saveTournament(savefile);
        savefile.close();
        tournament.prepareNextRound(mutationIntensity, engine, (tournamentSize - 1) / 2, tournamentSize - 1);
        tournament.addContestant(parent);
        while (tournament.size() < tournamentSize) {
            tournament.addContestant(Bot(parent, mutationIntensity, engine));
        }
    }
    return 0;
}
