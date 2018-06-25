#include <sstream>
#include "move.hpp"

std::ostream& operator<<(std::ostream& stream, const Move& move) {
	stream << (char) ('a' + move.from % 8) << 8 - move.from / 8 << "-" << (char) ('a' + move.to % 8) << 8 - move.to / 8;
	return stream;
}

Move Move::invert() const {
	Move result(63 - this->from, 63 - this->to, invertPiece(this->turn_to));
	return result;
}

bool Move::operator==(const Move& other) const {
	return this->from == other.from && this->to == other.to && this->turn_to == other.turn_to;
}
