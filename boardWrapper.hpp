#pragma once
#include "board.hpp"

struct BoardWrapper {
    Board<true> whiteBoard{};
    Board<false> blackBoard{};
    bool amIWhite{true};

    template <bool hasBeenWhite>
    BoardWrapper(Board<hasBeenWhite> board)
        : whiteBoard(board)
        , blackBoard(board)
        , amIWhite(hasBeenWhite) {}
};

bool operator<(const BoardWrapper& board1, const BoardWrapper& board2);
bool operator==(const BoardWrapper& board1, const BoardWrapper& board2);

std::ostream& operator<<(std::ostream& stream, const BoardWrapper& board);
