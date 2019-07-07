#include "move.hpp"

#include <sstream>

std::ostream& operator<<(std::ostream& stream, const Move& move) {
    std::ostringstream tmp;
    tmp << static_cast<char>(__builtin_ctzll(move.moveFrom) % 8 + 'a') << 8 - __builtin_ctzll(move.moveFrom) / 8;
    tmp << static_cast<char>(__builtin_ctzll(move.moveTo) % 8 + 'a') << 8 - __builtin_ctzll(move.moveTo) / 8;
    stream << tmp.str();
    return stream;
}
/*
std::ostream& operator<<(std::ostream& stream, const Move& move) {
        stream << (char) ('a' + __builtin_ctzll(move.moveFrom) % 8) << 8 - __builtin_ctzll(move.moveFrom) / 8 << "-" <<
(char) ('a' + __builtin_ctzll(move.moveTo) % 8) << 8 - __builtin_ctzll(move.moveTo) / 8; return stream;
}
*/
bool operator!=(const Move& l, const Move& r) {
    return l.moveFrom != r.moveFrom || l.moveTo != r.moveTo || l.turnFrom != r.turnFrom || l.turnTo != r.turnTo;
}

bool operator==(const Move& l, const Move& r) {
    return l.moveFrom == r.moveFrom && l.moveTo == r.moveTo && l.turnFrom == r.turnFrom && l.turnTo == r.turnTo;
}
