#include "tournament.hpp"
#include <algorithm>
#include <iostream>
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
                if (games.size() >= std::thread::hardware_concurrency() - 1) {
                    games.begin()->join();
                    games.pop_front();
                    if (loud) {
                        std::cout << results.front();
                    }
                    results.pop_front();
                }
                results.push_back(notPlayed);
                games.emplace_back(&Tournament::playGame<true>, this, it, jt, results.rbegin());
                // For single-threaded testing:
                // this->playGame<true>(it, jt, results.rbegin());
            }
            else {
                games.emplace_back([]() {});
                results.push_back(notPlayed);
            }
        }
        games.emplace_back([]() {});
        results.push_back(lineBreak);
    }
    while (!games.empty()) {
        games.begin()->join();
        games.pop_front();
        if (loud) {
            std::cout << results.front();
        }
        results.pop_front();
    }
}

std::ostream& operator<<(std::ostream& stream, const outcome& result) {
    switch (result) {
    case notPlayed: stream << '-'; break;
    case whiteWon: stream << '<'; break;
    case blackWon: stream << '>'; break;
    case draw: stream << '.'; break;
    case lineBreak: stream << '\n'; break;
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
