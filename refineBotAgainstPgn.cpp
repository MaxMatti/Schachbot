#include "bot.hpp"
#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sys/ioctl.h>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>

std::string demangle(const char* name) {
    int status = -4;
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
    return (status == 0) ? res.get() : name;
}

using MoveCache = std::map<Move, std::size_t>;

std::tuple<Board<true>, MoveCache, MoveCache> getCachedMoves(std::ifstream& cacheFile) {
    std::string line;
    std::getline(cacheFile, line);
    std::tuple<Board<true>, MoveCache, MoveCache> result;
    auto situation = line.substr(0, line.find(" "));
    std::get<0>(result) = Board<true>{situation};
    for (auto pos = situation.length() + 1; pos < line.length();) {
        auto midPos = line.find(" ", pos) + 1;
        auto endPos = line.find(" ", midPos) + 1;
        Move move{line.substr(pos, midPos - pos - 1)};
        if (isWhite(move.turnFrom)) {
            std::get<1>(result)[move] = std::stoll(line.substr(midPos, endPos - midPos - 1));
        }
        else if (isBlack(move.turnFrom)) {
            std::get<2>(result)[move] = std::stoll(line.substr(midPos, endPos - midPos - 1));
        }
        pos = endPos;
    }
    return result;
}

template <std::size_t depth, bool amIWhite>
std::vector<int> getMultipleScores(
    const std::vector<Bot>& contestants,
    Board<amIWhite> board,
    std::vector<int>& bestPreviousScores [[maybe_unused]],
    std::vector<int>& worstPreviousScores [[maybe_unused]]) {
    if (board.figures[board.OwnKing] == 0) {
        return std::vector<int>(
            contestants.size(), std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()));
    }
    if (board.figures[board.EnemyKing] == 0) {
        return std::vector<int>(
            contestants.size(), std::min(-std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
    }
    if constexpr (depth == 0) {
        std::vector<int> results(contestants.size(), 0);
        for (auto i : {board.OwnQueen, board.OwnRook, board.OwnBishop, board.OwnKnight, board.OwnPawn}) {
            auto tmp = __builtin_popcountll(board.figures[i]);
            for (std::size_t j = 0; j < contestants.size(); ++j) {
                results[j] += tmp * contestants[j].values[i] * contestants[j].values[board.OwnFigure];
            }
        }
        for (auto i : {board.EnemyQueen, board.EnemyRook, board.EnemyBishop, board.EnemyKnight, board.EnemyPawn}) {
            auto tmp = __builtin_popcountll(board.figures[i]);
            for (std::size_t j = 0; j < contestants.size(); ++j) {
                results[j] += tmp * contestants[j].values[i] * contestants[j].values[board.EnemyFigure];
            }
        }
        board.forEachValidMove([&](const Move& move) {
            for (std::size_t j = 0; j < contestants.size(); ++j) {
                results[j] += contestants[j].strengths[move.turnFrom] * contestants[j].strengths[board.OwnFigure];
                results[j] -= contestants[j].weaknesses[board.figureAt(move.moveTo)] *
                    contestants[j].weaknesses[board.EnemyFigure];
            }
        });
        Board<!amIWhite> invertedBoard(board);
        invertedBoard.forEachValidMove([&](const Move& move) {
            for (std::size_t j = 0; j < contestants.size(); ++j) {
                results[j] += contestants[j].strengths[move.turnFrom] * contestants[j].strengths[board.EnemyFigure];
                results[j] -=
                    contestants[j].weaknesses[board.figureAt(move.moveTo)] * contestants[j].weaknesses[board.OwnFigure];
            }
        });
        for (std::size_t j = 0; j < contestants.size(); ++j) {
            if constexpr (!amIWhite) {
                results[j] *= -1;
            }
        }
        return results;
    }
    else {
        std::vector<Board<!amIWhite>> situations;
        situations.reserve(64ul);
        // this number needs to be within the range set by getMove for bestScore.
        std::vector<int> bestScores(
            contestants.size(), std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()) + 1);
        board.forEachValidMove([&](const Move& move) { situations.push_back(board.applyMove(move)); });
        for (auto& it : situations) { /*
             bool skip = true;
             for (std::size_t i = 0; i < contestants.size(); ++i) {
                 // alpha-beta-pruning
                 if (bestScores[i] < bestPreviousScores[i]) {
                     skip = false;
                 }
             }
             if (skip) {
                 continue;
             }*/
            std::vector<int> currentScores = getMultipleScores<depth - 1>( //
                contestants,
                it,
                bestPreviousScores,
                worstPreviousScores);
            for (std::size_t i = 0; i < contestants.size(); ++i) {
                if (-currentScores[i] > bestScores[i]) {
                    bestScores[i] = -currentScores[i];
                }
            }
        }
        return bestScores;
    }
}

template <std::size_t depth, bool amIWhite>
std::vector<Move> getMultipleMoves(const std::vector<Bot>& contestants, Board<amIWhite> board) {

    std::vector<Move> bestMoves(contestants.size(), board.getFirstValidMove());
    std::vector<int> bestScores(
        contestants.size(), std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()));
    std::vector<int> worstScores(
        contestants.size(), -std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()));

    std::size_t moveCounter = 0;
    board.forEachValidMove([&](auto) { ++moveCounter; });
    std::cout << "moves: " << std::setw(3) << moveCounter << " ";

    board.forEachValidMove([&](auto move) {
        Board<!amIWhite> tmp = board.applyMove(move);
        auto currentScores = getMultipleScores<depth - 1>(contestants, tmp, bestScores, worstScores);
        std::cout << "." << std::flush;
        for (std::size_t i = 0; i < contestants.size(); ++i) {
            if (-currentScores[i] > bestScores[i]) {
                bestScores[i] = -currentScores[i];
                bestMoves[i] = move;
            }
        }
    });
    std::cout << "\n";
    return bestMoves;
}

template <class T>
auto calcMaxScore(T&& situations) {
    return std::accumulate(situations.begin(), situations.end(), 0ul, [](const auto& a, const auto& b) {
        auto max1 = std::get<1>(b).empty()
            ? 0ul
            : std::max_element(std::get<1>(b).begin(), std::get<1>(b).end(), [](auto a, auto b) {
                  return a.second < b.second;
              })->second;
        auto max2 = std::get<2>(b).empty()
            ? 0ul
            : std::max_element(std::get<2>(b).begin(), std::get<2>(b).end(), [](auto a, auto b) {
                  return a.second < b.second;
              })->second;
        return a + max1 + max2;
    });
}

void printColNumbers(std::size_t generationSize) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::cout << std::setw(std::to_string(generationSize).size() * 2 + 47) << "10";
    for (std::size_t i = 2; i < (w.ws_col - std::to_string(generationSize).size() * 2 - 27) / 10; ++i) {
        std::cout << std::setw(10) << i * 10;
    }
    std::cout << std::endl;
}

void saveCache(
    std::map<Bot, std::pair<std::size_t, std::size_t>> knownBots,
    std::map<Bot, std::map<Board<true>, Move>> whiteMoveCache,
    std::map<Bot, std::map<Board<false>, Move>> blackMoveCache,
    const std::string& filename) {

    std::ofstream out(filename.c_str(), std::ios_base::binary);

    auto vecSize = knownBots.size();
    out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
    for (auto& it : knownBots) {
        out.write(reinterpret_cast<const char*>(&it.first), sizeof(it.first));
        out.write(reinterpret_cast<const char*>(&it.second), sizeof(it.second));
    }
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
    out.close();
}

auto loadCache(const std::string& filename) -> std::tuple<
    std::map<Bot, std::pair<std::size_t, std::size_t>>,
    std::map<Bot, std::map<Board<true>, Move>>,
    std::map<Bot, std::map<Board<false>, Move>>> {

    std::ifstream in(filename.c_str(), std::ios_base::binary);

    std::map<Bot, std::pair<std::size_t, std::size_t>> knownBots;
    std::map<Bot, std::map<Board<true>, Move>> whiteMoveCache;
    std::map<Bot, std::map<Board<false>, Move>> blackMoveCache;
    // TODO(mstaff): error handling
    auto vecSize = knownBots.size();
    in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
    for (decltype(vecSize) i = 0; i < vecSize; ++i) {
        Bot currentBot;
        std::pair<std::size_t, std::size_t> currentScore;
        in.read(reinterpret_cast<char*>(&currentBot), sizeof(currentBot));
        in.read(reinterpret_cast<char*>(&currentScore), sizeof(currentScore));
        knownBots[currentBot] = currentScore;
    }
    std::string divider = "0000";
    in.read(divider.data(), 4);
    if (divider != "====") {
        knownBots.clear();
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return std::tuple{knownBots, whiteMoveCache, blackMoveCache};
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
    divider = "0000";
    in.read(divider.data(), 4);
    if (divider != "====") {
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return std::tuple{knownBots, whiteMoveCache, blackMoveCache};
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
    divider = "0000";
    in.read(divider.data(), 4);
    if (divider != "====") {
        whiteMoveCache.clear();
        blackMoveCache.clear();
        return std::tuple{knownBots, whiteMoveCache, blackMoveCache};
    }
    in.close();
    return std::tuple{knownBots, whiteMoveCache, blackMoveCache};
}

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    std::string scoreCacheFilename = "/tmp/scoreCache.txt";
    std::string botCacheFilename = "/tmp/botCache.txt";
    if (argc > 1) {
        scoreCacheFilename = argv[1];
    }
    if (argc > 2) {
        botCacheFilename = argv[2];
    }
    double mutationIntensity = 0.4;
    std::mt19937 engine;
    std::ifstream cacheFile(scoreCacheFilename);
    std::size_t winners = 10;
    std::size_t generationSize = 40;
    std::size_t startLines = 10;
    std::size_t lineIncrement = 10;
    std::size_t maxScore = 1;
    std::size_t minScore = 0;
    Bot newContestant{};
    auto [knownBots, whiteMoveCache, blackMoveCache] = loadCache(botCacheFilename);
    std::cout << std::setprecision(3) << "Loaded " << knownBots.size() << " known bots, "
              << std::accumulate(
                     whiteMoveCache.begin(),
                     whiteMoveCache.end(),
                     0ul,
                     [](const auto& sum, const auto& elem) { return sum + elem.second.size(); })
              << " white moves, "
              << std::accumulate(
                     blackMoveCache.begin(),
                     blackMoveCache.end(),
                     0ul,
                     [](const auto& sum, const auto& elem) { return sum + elem.second.size(); })
              << " black moves." << std::endl;
    std::vector<std::pair<Bot, std::pair<std::size_t, std::size_t>>> contestants;
    std::vector<std::tuple<Board<true>, MoveCache, MoveCache>> situations;
    situations.reserve(startLines + 10 * lineIncrement);
    for (std::size_t i = 0; i < startLines && !cacheFile.eof(); ++i) {
        situations.push_back(getCachedMoves(cacheFile));
    }
    maxScore = calcMaxScore(situations);
    while (true) {
        printColNumbers(generationSize);
        for (const auto& it : contestants) {
            if (knownBots.count(it.first)) {
                auto& knownPos = knownBots.at(it.first);
                if (knownPos.first < it.second.first) {
                    knownPos = it.second;
                }
            }
            else {
                knownBots.insert(it);
            }
        }
        saveCache(knownBots, whiteMoveCache, blackMoveCache, botCacheFilename);
        contestants.resize(0);
        contestants.reserve(knownBots.size());
        std::copy(knownBots.begin(), knownBots.end(), std::back_inserter(contestants));
        if (winners < contestants.size()) {
            std::partial_sort(
                contestants.begin(),
                contestants.begin() + winners,
                contestants.end(),
                [](const auto& a, const auto& b) {
                    // note: this is reversed because we want descending order
                    return a.second.second > b.second.second;
                });
            contestants.resize(winners);
        }
        contestants.reserve(generationSize);
        while (contestants.size() < winners) {
            contestants.emplace_back(Bot{Bot{}, mutationIntensity, engine}, std::pair{0ul, 0ul});
        }
        minScore = contestants.rbegin()->second.second;
        if (minScore > maxScore * 0.5) {
            situations.reserve(situations.size() + lineIncrement);
            for (std::size_t i = 0; i < lineIncrement; ++i) {
                situations.push_back(getCachedMoves(cacheFile));
            }
            maxScore = calcMaxScore(situations);
            mutationIntensity *= 0.5;
        }
        else {
            mutationIntensity = 1 - ((1 - mutationIntensity) * 0.75);
        }
        while (contestants.size() < generationSize) {
            contestants.emplace_back(
                Bot{contestants[contestants.size() % winners].first, mutationIntensity, engine}, std::pair{0ul, 0ul});
        }
        for (auto& it : contestants) {
            if (knownBots.count(it.first)) {
                it.second = knownBots.at(it.first);
            }
        }
        std::vector<Bot> currentGen;
        currentGen.reserve(generationSize);
        for (std::size_t i = 0; i < situations.size(); ++i) {
            const auto& whiteMoves = std::get<1>(situations[i]);
            const auto& blackMoves = std::get<2>(situations[i]);
            auto whiteBoard = Board<true>{std::get<0>(situations[i])};
            auto blackBoard = Board<false>{std::get<0>(situations[i])};
            currentGen.resize(0);
            for (auto& it : contestants) {
                // "<=" because a value of 0 means this bot has not evaluated situation 0
                if (it.second.first <= i) {
                    if ((whiteMoves.empty() ||
                         (whiteMoveCache.count(it.first) && whiteMoveCache.at(it.first).count(whiteBoard))) &&
                        (blackMoves.empty() ||
                         (blackMoveCache.count(it.first) && blackMoveCache.at(it.first).count(blackBoard)))) {

                        if (whiteMoveCache.count(it.first) && whiteMoveCache.at(it.first).count(whiteBoard)) {
                            auto whiteMove = whiteMoveCache.at(it.first).at(whiteBoard);
                            if (whiteMoves.count(whiteMove)) {
                                it.second.second += whiteMoves.at(whiteMove);
                            }
                        }
                        if (blackMoveCache.count(it.first) && blackMoveCache.at(it.first).count(blackBoard)) {
                            auto blackMove = blackMoveCache.at(it.first).at(blackBoard);
                            if (blackMoves.count(blackMove)) {
                                it.second.second += blackMoves.at(blackMove);
                            }
                        }
                    }
                    else {
                        currentGen.push_back(it.first);
                    }
                }
            }
            if (currentGen.size() > 0) {
                auto cont = contestants.begin();
                auto getNextContestant = [&](const auto& contestant) {
                    while (cont->first != contestant && cont != contestants.end()) {
                        ++cont;
                    }
                    cont = contestants.begin();
                    while (cont->first != contestant && cont != contestants.end()) {
                        ++cont;
                    }
                    if (cont == contestants.end()) {
                        std::cout << "\nError when evaluating " << cont->first << ".\n";
                        std::cout << "Contestants:\n";
                        for (const auto& it : contestants) {
                            std::cout << "    " << it.first << " -> (" << it.second.first << ", " << it.second.second
                                      << ")\n";
                        }
                        exit(1);
                    }
                };

                if (!whiteMoves.empty()) {
                    std::cout << "Size: " << std::setw(std::to_string(generationSize).size()) << currentGen.size()
                              << "/" << std::setw(std::to_string(generationSize).size()) << contestants.size()
                              << ", gen: " << std::setw(4) << i << ", white ";
                    cont = contestants.begin();
                    auto whiteBotMoves = getMultipleMoves<4>(currentGen, whiteBoard);
                    for (std::size_t j = 0; j < std::min(currentGen.size(), whiteBotMoves.size()); ++j) {
                        getNextContestant(currentGen[j]);
                        if (whiteMoves.count(whiteBotMoves[j])) {
                            cont->second.second += whiteMoves.at(whiteBotMoves[j]);
                        }
                        whiteMoveCache[currentGen[j]][whiteBoard] = whiteBotMoves[j];
                    }
                }

                if (!blackMoves.empty()) {
                    std::cout << "Size: " << std::setw(std::to_string(generationSize).size()) << currentGen.size()
                              << "/" << std::setw(std::to_string(generationSize).size()) << contestants.size()
                              << ", gen: " << std::setw(4) << i << ", black ";
                    cont = contestants.begin();
                    auto blackBotMoves = getMultipleMoves<4>(currentGen, blackBoard);
                    for (std::size_t j = 0; j < std::min(currentGen.size(), blackBotMoves.size()); ++j) {
                        getNextContestant(currentGen[j]);
                        if (blackMoves.count(blackBotMoves[j])) {
                            cont->second.second += blackMoves.at(blackBotMoves[j]);
                        }
                        blackMoveCache[currentGen[j]][blackBoard] = blackBotMoves[j];
                    }
                }

                cont = contestants.begin();
                for (const auto& it : currentGen) {
                    getNextContestant(it);
                    cont->second.first = i + 1;
                }
            }
            saveCache(knownBots, whiteMoveCache, blackMoveCache, botCacheFilename);
        }

        auto tmp = std::min_element(contestants.begin(), contestants.end(), [](const auto& a, const auto& b) {
                       return a.second.second < b.second.second;
                   })->second.second;
        std::cout << "MaxScore: " << maxScore << ", "
                  << "old MinScore: " << minScore << " (" << (100 * minScore / maxScore) << "%), "
                  << "new MinScore: " << tmp << " (" << (100 * tmp / maxScore) << "%), "
                  << "MutationIntensity: " << mutationIntensity << "\n";

        auto minLevel = std::min_element(knownBots.begin(), knownBots.end(), [](const auto& a, const auto& b) {
                            return a.second.first < b.second.first;
                        })->second.first;
        auto maxLevel = std::max_element(knownBots.begin(), knownBots.end(), [](const auto& a, const auto& b) {
                            return a.second.first < b.second.first;
                        })->second.first;
        std::cout << "Known Bots: " << knownBots.size() << " (level " << minLevel << "-" << maxLevel << ")\n";

        for (const auto& it : contestants) {
            std::cout << it.first << " -> (" << it.second.first << ", " << it.second.second << ")" << std::endl;
        }
    }
    cacheFile.close();
    return 0;
}
