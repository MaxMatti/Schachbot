#include "tournament.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <thread>

Tournament::Tournament(const Tournament& previous, const float& mutationIntensity, std::mt19937& generator)
    : contestants(previous.contestants) {
    std::sort(this->contestants.begin(), this->contestants.end(), [](auto& a, auto& b) {
        // reversed comparison because we want descending order
        return std::get<1>(a) > std::get<1>(b);
    });
    this->contestants.reserve(this->contestants.size());
    this->contestants.resize(this->contestants.size() / 2);
    for (std::pair<Bot, int>& contestant : this->contestants) {
        this->contestants.emplace_back(Bot(std::get<0>(contestant), mutationIntensity, generator), 0);
        std::get<1>(contestant) = 0;
    }
}

bool Tournament::addContestant(const Bot& newContestant) {
    if (std::find_if(this->contestants.begin(), this->contestants.end(), [&](auto a) {
            return std::get<0>(a) == newContestant;
        }) != this->contestants.end()) {
        return false;
    }
    this->contestants.emplace_back(newContestant, 0);
    return true;
}

bool Tournament::addContestant(Bot&& newContestant) {
    if (std::find_if(this->contestants.begin(), this->contestants.end(), [&](auto a) {
            return std::get<0>(a) == newContestant;
        }) != this->contestants.end()) {
        return false;
    }
    this->contestants.emplace_back(std::move(newContestant), 0);
    return true;
}

void Tournament::evaluate(const bool loud) {
    std::list<std::thread> games;
    std::list<outcome> results;
    for (auto it = this->contestants.begin(); it != this->contestants.end(); ++it) {
        for (auto jt = this->contestants.begin(); jt != this->contestants.end(); ++jt) {
            if (it != jt) {
                // Might introduce situations where only one thread is active when that game takes longer than the later
                // ones however a proper solution would take too long to implement.Might even remove this at some point.
                /*if (games.size() >= std::thread::hardware_concurrency() - 1) {
                    games.begin()->join();
                    games.pop_front();
                    if (loud) {
                        std::cout << results.front();
                    }
                    results.pop_front();
                }
                results.push_back(notPlayed);
                games.emplace_back(&Tournament::playGame<true>, this, it, jt, results.rbegin());*/
                // For single-threaded testing:
                results.push_back(notPlayed);
                this->playGame(it, jt, results.rbegin());
            }
            else {
                // games.emplace_back([]() {});
                results.push_back(notPlayed);
            }
        }
        // games.emplace_back([]() {});
        results.push_back(lineBreak);
    }
    /*while (!games.empty()) {
        games.begin()->join();
        games.pop_front();
        if (loud) {
            std::cout << results.front();
        }
        results.pop_front();
    }*/
    if (loud) {
        for (const auto& it : results) {
            std::cout << it;
        }
    }
}

void Tournament::prepareNextRound(
    const float& mutationIntensity,
    std::mt19937& generator,
    const std::size_t winners,
    const std::size_t generationSize) {

    std::partial_sort(contestants.begin(), contestants.begin() + winners, contestants.end(), [](auto x, auto y) {
        return x.second > y.second; // note: this is reversed because we want descending order
    });
    contestants.resize(generationSize);
    for (auto i = 0ul; i < winners; ++i) {
        contestants[i].second = 0;
    }
    for (auto i = winners; i < generationSize; ++i) {
        contestants[i] = std::pair(Bot(contestants[i % winners].first, mutationIntensity, generator), 0);
    }
}

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
    std::map<Board<true>, std::size_t> currentBoardCounter;
    std::map<Board<false>, std::size_t> reverseBoardCounter;

    while (true) {
        if (reverseSituation.isThreatened(reverseSituation.figures[BlackKing])) {
            bot1->second += 3;
            *result = whiteWon;
            return;
        }
        if (currentSituation.getFirstValidMove() == Move{} || currentBoardCounter[currentSituation] > 10) {
            bot1->second += 1;
            bot2->second += 1;
            *result = draw;
            return;
        }
        whiteMove = bot1->first.getMove<4, false>(currentSituation);
        reverseSituation = currentSituation.applyMove(whiteMove);
        ++reverseBoardCounter[reverseSituation];
        if (currentSituation.figures[BlackKing] == 0ul) {
            bot1->second += 3;
            *result = whiteWon;
            return;
        }
        if (currentSituation.isThreatened(currentSituation.figures[WhiteKing])) {
            bot2->second += 3;
            *result = blackWon;
            return;
        }
        if (reverseSituation.getFirstValidMove() == Move{} || reverseBoardCounter[reverseSituation] > 10) {
            bot1->second += 1;
            bot2->second += 1;
            *result = draw;
            return;
        }
        blackMove = bot2->first.getMove<4, false>(reverseSituation);
        currentSituation = reverseSituation.applyMove(blackMove);
        ++currentBoardCounter[currentSituation];
        if (currentSituation.figures[WhiteKing] == 0ul) {
            bot2->second += 3;
            *result = blackWon;
            return;
        }
    }
    __builtin_unreachable();
}

void Tournament::saveTournament(std::ofstream& out) const {
    auto vecSize = contestants.size();
    out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
    out.write(reinterpret_cast<const char*>(contestants.data()), sizeof(*contestants.data()) * vecSize);
}

void Tournament::loadTournament(std::ifstream& in) {
    auto vecSize = contestants.size();
    in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
    contestants.resize(vecSize);
    in.read(reinterpret_cast<char*>(contestants.data()), sizeof(*contestants.data()) * vecSize);
}

std::ostream& operator<<(std::ostream& stream, const outcome& result) {
    switch (result) {
    case notPlayed: stream << '-'; break;
    case whiteWon: stream << '<'; break;
    case blackWon: stream << '>'; break;
    case draw: stream << '.'; break;
    case lineBreak: stream << std::endl; break;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Tournament& tournament) {
    stream << "Tournament:\n";
    for (auto contestant : tournament.contestants) {
        stream << std::get<0>(contestant) << " - " << std::get<1>(contestant) << "\n";
    }
    return stream;
}
