#include "piece.hpp"
#include <sstream>

std::ostream& operator<<(std::ostream& stream, const piece& obj) {
    stream << getChessChar(obj);
    return stream;
}

std::string printPos(std::uint64_t pos) {
    std::string result;
    for (auto i = 1ul; i > 0ul; i <<= 8) {
        // TODO(mstaff): there's probably a more elegant solution than manual unrolling but I can't be bothered to think
        // of one right now.
        result += (pos & i) ? "1" : "0";
        result += (pos & (i << 1)) ? "1" : "0";
        result += (pos & (i << 2)) ? "1" : "0";
        result += (pos & (i << 3)) ? "1" : "0";
        result += (pos & (i << 4)) ? "1" : "0";
        result += (pos & (i << 5)) ? "1" : "0";
        result += (pos & (i << 6)) ? "1" : "0";
        result += (pos & (i << 7)) ? "1" : "0";
        result += "\n";
    }
    return result;
}
