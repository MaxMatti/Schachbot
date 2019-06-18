#include "move.hpp"

#include <sstream>

std::ostream& operator<<(std::ostream& stream, const Move& move) {
	std::ostringstream tmp;
	tmp << static_cast<char>(__builtin_ctzll(move.move_from) % 8 + 'a') << 8 - __builtin_ctzll(move.move_from) / 8;
	tmp << static_cast<char>(__builtin_ctzll(move.move_to) % 8 + 'a') << 8 - __builtin_ctzll(move.move_to) / 8;
	stream << tmp.str();
	return stream;
}
/*
std::ostream& operator<<(std::ostream& stream, const Move& move) {
	stream << (char) ('a' + __builtin_ctzll(move.move_from) % 8) << 8 - __builtin_ctzll(move.move_from) / 8 << "-" << (char) ('a' + __builtin_ctzll(move.move_to) % 8) << 8 - __builtin_ctzll(move.move_to) / 8;
	return stream;
}
*/
bool Move::operator==(const Move& other) const {
	return move_from == other.move_from && move_to == other.move_to && turn_to == other.turn_to;
}
