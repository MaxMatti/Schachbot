#include "board.hpp"

#pragma once

class Move {
private:
	unsigned char from;
	unsigned char to;
	piece turn_to;
public:
	Move() : from(0), to(0), turn_to(None) {}
	Move(unsigned char from, unsigned char to, piece turn_to = None) : from(from), to(to), turn_to(turn_to) {}
	Move invert() const;
	bool operator==(const Move& other) const;

	friend Board;
	friend std::ostream& operator<<(std::ostream& stream, const Move& move);
};

std::ostream& operator<<(std::ostream& stream, const Move& move);
