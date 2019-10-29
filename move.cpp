#include "move.hpp"

#include <sstream>

std::ostream& operator<<(std::ostream& stream, const Move& move) {
    std::ostringstream tmp;
    tmp << move.turnFrom;
    tmp << static_cast<char>(__builtin_ctzll(move.moveFrom) % 8 + 'a') << 8 - __builtin_ctzll(move.moveFrom) / 8;
    tmp << static_cast<char>(__builtin_ctzll(move.moveTo) % 8 + 'a') << 8 - __builtin_ctzll(move.moveTo) / 8;
    if (move.turnFrom != move.turnTo) {
        tmp << move.turnTo;
    }
    stream << tmp.str();
    return stream;
}
/*
std::ostream& operator<<(std::ostream& stream, const Move& move) {
        stream << (char) ('a' + __builtin_ctzll(move.moveFrom) % 8) << 8 - __builtin_ctzll(move.moveFrom) / 8 << "-" <<
(char) ('a' + __builtin_ctzll(move.moveTo) % 8) << 8 - __builtin_ctzll(move.moveTo) / 8; return stream;
}
*/

bool isValidMove(Move move, std::uint64_t obstacles, std::uint64_t enPassent) {
    if (isKing(move.turnFrom)) {
        return isValidKingMove(move.moveFrom, move.moveTo);
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
