#pragma once

#include "bot.hpp"
#include <deque>
#include <map>
#include <vector>

template <bool amIWhite>
struct DynamicBot : Bot {
    using OptionalPath = std::tuple<Move, Board<amIWhite>, std::size_t>;
    std::map<int, std::deque<OptionalPath>> storage;

    template <std::size_t depth>
    void optimizeStorage(std::size_t runs) {
        auto bestItems = std::find_if(storage.rbegin(), storage.rend(), [](auto queue) { return !queue.empty(); });
        for (std::size_t i = 0; i < runs; ++i) {
            if (bestItems.empty()) {
                bestItems = std::find_if(bestItems, storage.rend(), [](auto queue) { return !queue.empty(); });
            }
            if (bestItems == storage.rend()) {
                bestItems = std::find_if(storage.begin(), storage.rend(), [](auto queue) { return !queue.empty(); });
            }
            if (bestItems == storage.rend()) {
                return;
            }
            auto& currentItem = bestItems->front();
            auto& currentBoard = std::get<Board<amIWhite>>(currentItem);
            currentBoard.forEachValidMove([&](auto currentMove) {
                Board<!amIWhite> otherBoard = currentBoard.applyMove(currentMove);
                otherBoard.forEachValidMove([&](auto otherMove) {
                    Board<amIWhite> boardToInsert = otherBoard.applyMove(otherMove);
                    const int bestScore{std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max())};
                    storage[getScore<depth>(boardToInsert, -bestScore, bestScore)].emplace_back(
                        std::get<0>(currentItem), boardToInsert, std::get<2>(currentItem) + 1);
                });
            });
            bestItems->pop_front();
        }
    }

    template <std::size_t depth>
    void optimizeStorageForDuration(std::chrono::steady_clock::duration duration) {
        auto start = std::chrono::steady_clock::now();
        std::size_t totalRuns = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        optimizeStorage<depth>(totalRuns);
        auto timePassed = std::chrono::steady_clock::now() - start;
        while (timePassed < duration) {
            optimizeStorage<depth>(totalRuns * (duration - timePassed) / timePassed / 2);
            timePassed = std::chrono::steady_clock::now() - start;
        }
    }

    template <std::size_t depth>
    std::pair<Move, std::size_t> getTimedMove(
        Board<amIWhite> inputBoard, std::chrono::steady_clock::duration duration) {
        storage.clear();
        inputBoard.forEachValidMove([&](auto currentMove) {
            Board<!amIWhite> otherBoard = inputBoard.applyMove(currentMove);
            otherBoard.forEachValidMove([&](auto otherMove) {
                Board<amIWhite> boardToInsert = otherBoard.applyMove(otherMove);
                const int bestScore{std::max(std::numeric_limits<int>::min(), -std::numeric_limits<int>::max())};
                storage[getScore<depth>(boardToInsert, -bestScore, bestScore)].emplace_back(
                    currentMove, boardToInsert, 1);
            });
        });
        optimizeStorageForDuration<depth>(duration);
        auto bestItem =
            std::find_if(storage.rbegin(), storage.rend(), [](auto queue) { return !queue.empty(); }).front();
        return std::pair(std::get<0>(bestItem), std::get<2>(bestItem));
    }
};
