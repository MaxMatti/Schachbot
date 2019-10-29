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
