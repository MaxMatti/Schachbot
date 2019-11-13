#pragma once

#include "boardWrapper.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <limits>
#include <list>
#include <numeric>
#include <random>
#include <tuple>

template <class A>
struct ArraySizeImpl;

template <class T, std::size_t _N>
struct ArraySizeImpl<std::array<T, _N>> {
    static constexpr std::size_t N = _N;
};

template <class A>
constexpr std::size_t arraySize() {
    return ArraySizeImpl<A>::N;
}

using time_point = std::chrono::steady_clock::time_point;
using duration = std::chrono::steady_clock::duration;

/*static std::array<size_t, 64> moveCounter{
    0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul,
    0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul,
    0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul};*/

std::string printDuration(duration time);
std::string printDurationSince(time_point start);

inline auto getMsSince(time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
}

struct Bot {
    std::array<int, 16> values;
    std::array<int, 16> strengths;
    std::array<int, 16> weaknesses;
    Bot();
    Bot(std::array<int, 16> new_values)
        : values(new_values) {}
    Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator);

    template <std::size_t depth, bool loud>
    Move getMove(BoardWrapper board);

    template <std::size_t depth, bool loud, bool amIWhite>
    Move getMove(Board<amIWhite> board);

    template <std::size_t depth, bool amIWhite>
    int getScore(Board<amIWhite> board, int bestPreviousScore, int worstPreviousScore);

    std::int64_t counter{0};
};

std::ostream& operator<<(std::ostream& stream, const Bot& bot);
bool operator<(const Bot& bot1, const Bot& bot2);
bool operator==(const Bot& bot1, const Bot& bot2);

template <std::size_t depth, bool loud>
Move Bot::getMove(BoardWrapper board) {
    if (board.amIWhite) {
        return getMove<depth, loud>(board.whiteBoard);
    }
    else {
        return getMove<depth, loud>(board.blackBoard);
    }
}

template <std::size_t depth, bool loud, bool amIWhite>
Move Bot::getMove(Board<amIWhite> board) {
    auto start [[maybe_unused]] = std::chrono::steady_clock::now();
    Move bestMove = board.getFirstValidMove();
    // This number needs to be converted between positive and negative without any loss, thus the formula.
    int bestScore{std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max())};
    board.forEachValidMove([&](auto move) {
        Board<!amIWhite> tmp = board.applyMove(move);
        int currentScore = -getScore<depth - 1>(
            tmp, -bestScore, std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()));
        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestMove = move;
        }
    });
    if constexpr (loud) {
        std::cout << "Chose " << bestMove << " in " << getMsSince(start) << "ms\n";
    }
    return bestMove;
}

template <std::size_t depth, bool amIWhite>
int Bot::getScore(
    Board<amIWhite> board, int bestPreviousScore [[maybe_unused]], int worstPreviousScore [[maybe_unused]]) {
    if constexpr (depth == 0) {
        ++counter;
    }
    if (board.figures[board.OwnKing] == 0) {
        return std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max());
    }
    if (board.figures[board.EnemyKing] == 0) {
        return std::min(-std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    }
    if constexpr (depth == 0) {
        int result{0};
        static_assert(arraySize<decltype(values)>() >= arraySize<decltype(board.figures)>());
        for (auto i : {board.OwnQueen, board.OwnRook, board.OwnBishop, board.OwnKnight, board.OwnPawn}) {
            result += __builtin_popcountll(board.figures[i]) * values[i] * values[board.OwnFigure];
        }
        for (auto i : {board.EnemyQueen, board.EnemyRook, board.EnemyBishop, board.EnemyKnight, board.EnemyPawn}) {
            result += __builtin_popcountll(board.figures[i]) * values[i] * values[board.EnemyFigure];
        }
        board.forEachValidMove([&](const Move& move) {
            result += strengths[move.turnFrom] * strengths[board.OwnFigure];
            result -= weaknesses[board.figureAt(move.moveTo)] * weaknesses[board.EnemyFigure];
        });
        Board<!amIWhite> invertedBoard(board);
        invertedBoard.forEachValidMove([&](const Move& move) {
            result += strengths[move.turnFrom] * strengths[board.EnemyFigure];
            result -= weaknesses[board.figureAt(move.moveTo)] * weaknesses[board.OwnFigure];
        });
        return result;
    }
    else {
        std::vector<std::tuple<Move, Board<amIWhite>, int, int>> situations;
        situations.reserve(64ul);
        // this number needs to be within the range set by getMove for bestScore.
        int bestScore{std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()) + 1};
        int bestPossibleScore{std::min(-std::numeric_limits<int>::min(), std::numeric_limits<int>::max())};
        int shallowScore = getScore<0>(board, bestPreviousScore, worstPreviousScore);
        board.forEachValidMove([&](const Move& move) { situations.push_back({move, board.applyMove(move), 0, 0}); });
        //++moveCounter[std::min(situations.size(), 64ul)];
        for (auto& it : situations) {
            if (std::get<1>(it).figures[board.EnemyKing] == 0ul) {
                return bestPossibleScore;
            }
            std::get<2>(it) = -getScore<0>(std::get<1>(it), -worstPreviousScore, -bestPreviousScore);
        }
        constexpr const static size_t pruningCounter = 10;
        if (situations.size() > pruningCounter) {
            std::partial_sort(
                situations.begin(), situations.begin() + pruningCounter, situations.end(), [](auto a, auto b) {
                    return std::get<2>(a) > std::get<2>(b);
                });
            if constexpr (depth > 3) {
                for (auto it = situations.begin(); it < situations.end(); ++it) {
                    if (std::get<1>(*it).isThreatened(std::get<1>(*it).figures[board.OwnKing])) {
                        std::get<2>(*it) = -bestPossibleScore;
                    }
                    int doubleMovePruningScore = getScore<0>(std::get<1>(*it), bestPreviousScore, worstPreviousScore);
                    if (doubleMovePruningScore < shallowScore * 2 + 100) {
                        std::get<2>(*it) = -bestPossibleScore / 2;
                    }
                }
                std::partial_sort(
                    situations.begin(), situations.begin() + pruningCounter, situations.end(), [](auto a, auto b) {
                        return std::get<2>(a) > std::get<2>(b);
                    });
            }
        }
        for (auto& it : situations) {
            // alpha-beta-pruning
            if (bestScore >= bestPreviousScore) {
                break;
            }
            Board<!amIWhite> tmp{std::get<1>(it)};
            int currentScore = -getScore<depth - 1>(tmp, -worstPreviousScore, -bestPreviousScore);
            if (currentScore > bestScore) {
                bestScore = currentScore;
            }
        }
        return bestScore; /*
         board.forEachValidMove([&](const Move& move) {
             // alpha-beta-pruning
             if (bestScore >= bestPreviousScore) {
                 return;
             }
             // double-move-pruning and avoiding to run into check
             if constexpr (depth > 3) {
                 Board<amIWhite> doubleMovePruningBoard = board.applyMove(move);
                 if (doubleMovePruningBoard.isThreatened(doubleMovePruningBoard.figures[WhiteKing])) {
                     return;
                 }
                 int doubleMovePruningScore =
                     getScore<depth % 2 + 2>(doubleMovePruningBoard, bestPreviousScore, worstPreviousScore);
                 if (doubleMovePruningScore < shallowScore * 2 + 100) {
                     return;
                 }
             }
             Board<!amIWhite> tmp = board.applyMove(move);
             int currentScore = -getScore<depth - 1>(tmp, -worstPreviousScore, -bestPreviousScore);
             if (currentScore > bestScore) {
                 bestScore = currentScore;
             }
         });
         return bestScore;*/
    }
}
