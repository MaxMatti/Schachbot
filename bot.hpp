#pragma once

#include "board.hpp"
#include "move.hpp"
#include "piece.hpp"
#include <chrono>
#include <iomanip>
#include <limits>
#include <numeric>
#include <random>

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

std::string printDuration(duration time);
std::string printDurationSince(time_point start);

inline auto getMsSince(time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
}

struct Bot {
    std::array<int, 15> values;
    std::vector<std::size_t> statistics;
    Bot();
    Bot(std::array<int, 15> new_values)
        : values(new_values) {}
    Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator);

    template <std::size_t depth, bool loud, bool amIWhite>
    Move getMove(Board<amIWhite> board);

    template <std::size_t depth, bool amIWhite>
    int getScore(Board<amIWhite> board, int bestPreviousScore, int worstPreviousScore);

    std::string printStats(duration time) const;
    void resetStats();
};

std::ostream& operator<<(std::ostream& stream, const Bot& bot);
bool operator<(const Bot& bot1, const Bot& bot2);
bool operator==(const Bot& bot1, const Bot& bot2);

template <std::size_t depth, bool loud, bool amIWhite>
Move Bot::getMove(Board<amIWhite> board) {
    auto start [[maybe_unused]] = std::chrono::steady_clock::now();
    statistics.clear();
    statistics.resize(depth);
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
        /*if constexpr (loud) {
            std::cout << move << ": " << std::setw(12) << currentScore << "\n";
        }*/
    });
    if constexpr (loud) {
        std::cout << "Chose " << bestMove << " in " << getMsSince(start) << "ms\nStats:\n"
                  << printStats(std::chrono::steady_clock::now() - start);
    }
    return bestMove;
}

template <std::size_t depth, bool amIWhite>
int Bot::getScore(
    Board<amIWhite> board, int bestPreviousScore [[maybe_unused]], int worstPreviousScore [[maybe_unused]]) {
    ++statistics[depth];
    if (__builtin_popcountll(board.figures[OwnKing]) == 0) {
        return std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max());
    }
    if (__builtin_popcountll(board.figures[EnemyKing]) == 0) {
        return std::min(-std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    }
    if constexpr (depth == 0) {
        int result{0};
        static_assert(arraySize<decltype(values)>() >= arraySize<decltype(board.figures)>());
        for (auto i : {OwnQueen, OwnRook, OwnBishop, OwnKnight, OwnPawn}) {
            result += __builtin_popcountll(board.figures[i]) * values[i] * values[OwnFigure];
        }
        for (auto i : {EnemyQueen, EnemyRook, EnemyBishop, EnemyKnight, EnemyPawn}) {
            result += __builtin_popcountll(board.figures[i]) * values[i] * values[EnemyFigure];
        }
        board.forEachValidMove([&](const Move& move) { result += values[move.turnFrom]; });
        Board<!amIWhite> invertedBoard(board);
        invertedBoard.forEachValidMove([&](const Move& move) { result += values[move.turnFrom]; });
        return result;
    }
    else {
        // this number needs to be within the range set by getMove for bestScore.
        int bestScore{std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max()) + 1};
        int shallowScore = getScore<0>(board, bestPreviousScore, worstPreviousScore);
        board.forEachValidMove([&](const Move& move) {
            // alpha-beta-pruning
            if (bestScore >= bestPreviousScore) {
                return;
            }
            // double-move-pruning
            if constexpr (depth > 3) {
                Board<amIWhite> doubleMovePruningBoard = board.applyMove(move);
                int doubleMovePruningScore =
                    getScore<depth - 4>(doubleMovePruningBoard, bestPreviousScore, worstPreviousScore);
                if (doubleMovePruningScore < shallowScore) {
                    return;
                }
            }
            Board<!amIWhite> tmp = board.applyMove(move);
            int currentScore = -getScore<depth - 1>(tmp, -worstPreviousScore, -bestPreviousScore);
            if (currentScore > bestScore) {
                bestScore = currentScore;
            }
        });
        return bestScore;
    }
}
