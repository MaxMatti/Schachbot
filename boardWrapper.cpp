#include "boardWrapper.hpp"

bool operator<(const BoardWrapper& board1, const BoardWrapper& board2) {
    return board1.amIWhite < board2.amIWhite ||
        (board1.amIWhite == board2.amIWhite &&
         (board1.amIWhite ? board1.whiteBoard < board2.whiteBoard : board1.blackBoard < board2.blackBoard));
}

bool operator==(const BoardWrapper& board1, const BoardWrapper& board2) {
    return board1.amIWhite == board2.amIWhite &&
        (board1.amIWhite ? board1.whiteBoard == board2.whiteBoard : board1.blackBoard == board2.blackBoard);
}

std::ostream& operator<<(std::ostream& stream, const BoardWrapper& board) {
    if (board.amIWhite) {
        stream << board.whiteBoard;
    }
    else {
        stream << board.blackBoard;
    }
    return stream;
}
