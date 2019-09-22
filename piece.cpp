#include "piece.hpp"
#include <sstream>

std::ostream& operator<<(std::ostream& stream, const piece& obj) {
	stream << getChessChar(obj);
	return stream;
}
