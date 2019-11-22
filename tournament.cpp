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
        if (whiteMoveCache.count(bot1->first) && whiteMoveCache.at(bot1->first).count(currentSituation)) {
            whiteMove = whiteMoveCache.at(bot1->first).at(currentSituation);
        }
        else {
            whiteMove = bot1->first.getMove<4, false>(currentSituation);
            whiteMoveCache[bot1->first][currentSituation] = whiteMove;
        }
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
        if (blackMoveCache.count(bot2->first) && blackMoveCache.at(bot2->first).count(reverseSituation)) {
            blackMove = blackMoveCache.at(bot2->first).at(reverseSituation);
        }
        else {
            blackMove = bot2->first.getMove<4, false>(reverseSituation);
            blackMoveCache[bot2->first][reverseSituation] = blackMove;
        }
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
    out.write("====", 4);
    vecSize = whiteMoveCache.size();
    out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
    for (auto& it : whiteMoveCache) {
        out.write(reinterpret_cast<const char*>(&it.first), sizeof(it.first));
        auto subVecSize = it.second.size();
        out.write(reinterpret_cast<const char*>(&subVecSize), sizeof(subVecSize));
        for (auto& jt : it.second) {
            out.write(reinterpret_cast<const char*>(&jt.first), sizeof(jt.first));
            out.write(reinterpret_cast<const char*>(&jt.second), sizeof(jt.second));
        }
    }
    out.write("====", 4);
    vecSize = blackMoveCache.size();
    out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
    for (auto& it : blackMoveCache) {
        out.write(reinterpret_cast<const char*>(&it.first), sizeof(it.first));
        auto subVecSize = it.second.size();
        out.write(reinterpret_cast<const char*>(&subVecSize), sizeof(subVecSize));
        for (auto& jt : it.second) {
            out.write(reinterpret_cast<const char*>(&jt.first), sizeof(jt.first));
            out.write(reinterpret_cast<const char*>(&jt.second), sizeof(jt.second));
        }
    }
    out.write("====", 4);
}

void Tournament::loadTournament(std::ifstream& in) {
    // TODO(mstaff): error handling
    auto vecSize = contestants.size();
    in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
    contestants.resize(vecSize);
    in.read(reinterpret_cast<char*>(contestants.data()), sizeof(*contestants.data()) * vecSize);
    std::string divider = "0000";
    in.read(divider.data(), 4);
    if (divider != "====") {
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return;
    }
    vecSize = whiteMoveCache.size();
    in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
    for (decltype(vecSize) i = 0; i < vecSize; ++i) {
        decltype(vecSize) subVecSize;
        Bot currentBot;
        in.read(reinterpret_cast<char*>(&currentBot), sizeof(currentBot));
        in.read(reinterpret_cast<char*>(&subVecSize), sizeof(subVecSize));
        for (decltype(subVecSize) j = 0; j < subVecSize; ++j) {
            Board<true> currentBoard;
            Move currentMove;
            in.read(reinterpret_cast<char*>(&currentBoard), sizeof(currentBoard));
            in.read(reinterpret_cast<char*>(&currentMove), sizeof(currentMove));
            whiteMoveCache[currentBot][currentBoard] = currentMove;
        }
    }
    in.read(divider.data(), 4);
    if (divider != "====") {
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return;
    }
    vecSize = blackMoveCache.size();
    in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
    for (decltype(vecSize) i = 0; i < vecSize; ++i) {
        decltype(vecSize) subVecSize;
        Bot currentBot;
        in.read(reinterpret_cast<char*>(&currentBot), sizeof(currentBot));
        in.read(reinterpret_cast<char*>(&subVecSize), sizeof(subVecSize));
        for (decltype(subVecSize) j = 0; j < subVecSize; ++j) {
            Board<true> currentBoard;
            Move currentMove;
            in.read(reinterpret_cast<char*>(&currentBoard), sizeof(currentBoard));
            in.read(reinterpret_cast<char*>(&currentMove), sizeof(currentMove));
            blackMoveCache[currentBot][currentBoard] = currentMove;
        }
    }
    in.read(divider.data(), 4);
    if (divider != "====") {
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return;
    }
}

std::string Tournament::extraInfo() const {
    std::ostringstream tmp;
    tmp << std::accumulate(
               whiteMoveCache.begin(),
               whiteMoveCache.end(),
               0ul,
               [](const auto& sum, const auto& cacheLine) { return sum + cacheLine.second.size(); })
        << " white moves cached for " << whiteMoveCache.size() << " moves:\n";
    for (auto& it : whiteMoveCache) {
        tmp << it.first << " - " << it.second.size() << "\n";
    }
    tmp << std::accumulate(
               blackMoveCache.begin(),
               blackMoveCache.end(),
               0ul,
               [](const auto& sum, const auto& cacheLine) { return sum + cacheLine.second.size(); })
        << " black moves cached for " << blackMoveCache.size() << " moves:\n";
    for (auto& it : blackMoveCache) {
        tmp << it.first << " - " << it.second.size() << "\n";
    }
    return tmp.str();
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
