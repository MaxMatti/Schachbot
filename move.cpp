#include "move.hpp"

#include <sstream>

Move::Move(std::string movestr) {
    assert(movestr.length() == 6);
    moveFrom = 1ul << ((movestr[1] - 'a') + 8 * ('8' - movestr[2]));
    moveTo = 1ul << ((movestr[3] - 'a') + 8 * ('8' - movestr[4]));
    turnFrom = getPiece(movestr[0]);
    turnTo = getPiece(movestr[5]);
}

std::ostream& operator<<(std::ostream& stream, const Move& move) {
    std::ostringstream tmp;
    tmp << move.turnFrom;
    tmp << static_cast<char>(__builtin_ctzll(move.moveFrom) % 8 + 'a') << (8 - __builtin_ctzll(move.moveFrom) / 8);
    tmp << static_cast<char>(__builtin_ctzll(move.moveTo) % 8 + 'a') << (8 - __builtin_ctzll(move.moveTo) / 8);
    if (move.turnFrom != move.turnTo) {
        tmp << move.turnTo;
    }
    stream << tmp.str();
    return stream;
}

bool isValidMove(
    Move move,
    std::uint64_t obstacles,
    bool castling1Enabled,
    bool castling2Enabled,
    bool castling3Enabled,
    bool castling4Enabled,
    std::uint64_t enPassent) {
    if (isKing(move.turnFrom)) {
        if (move.turnFrom == WhiteKing) {
            return isValidKingMove<true>(
                move.moveFrom, move.moveTo, castling1Enabled, castling2Enabled, castling3Enabled, castling4Enabled);
        }
        else if (move.turnFrom == BlackKing) {
            return isValidKingMove<false>(
                move.moveFrom, move.moveTo, castling1Enabled, castling2Enabled, castling3Enabled, castling4Enabled);
        }
    }
    else if (isQueen(move.turnFrom)) {
        return isValidQueenMove(move.moveFrom, move.moveTo, obstacles);
    }
    else if (isRook(move.turnFrom)) {
        return isValidRookMove(move.moveFrom, move.moveTo, obstacles);
    }
    else if (isBishop(move.turnFrom)) {
        return isValidBishopMove(move.moveFrom, move.moveTo, obstacles);
    }
    else if (isKnight(move.turnFrom)) {
        return isValidKnightMove(move.moveFrom, move.moveTo, obstacles);
    }
    else if (isPawn(move.turnFrom)) {
        if (move.turnFrom == WhitePawn) {
            return isValidPawnMove<true>(move.moveFrom, move.moveTo, obstacles, enPassent);
        }
        else if (move.turnFrom == BlackPawn) {
            return isValidPawnMove<false>(move.moveFrom, move.moveTo, obstacles, enPassent);
        }
    }
    assert(false && "Cannot test validity of moves for this type of piece!");
    return false;
}
