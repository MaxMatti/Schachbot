#include <algorithm>
#include <iostream>
#include <string>
#include "board.hpp"
#include "move.hpp"

std::string getChessSymbol(piece type) {
	switch (type) {
		case None:
		return " ";
		case EnemyKing:
		return "\033[1;33m\u265A\033[0m";
		case EnemyQueen:
		return "\033[1;33m\u265B\033[0m";
		case EnemyRook:
		return "\033[1;33m\u265C\033[0m";
		case EnemyBishop:
		return "\033[1;33m\u265D\033[0m";
		case EnemyKnight:
		return "\033[1;33m\u265E\033[0m";
		case EnemyPawn:
		return "\033[1;33m\u265F\033[0m";
		case OwnKing:
		return "\033[1;37m\u265A\033[0m";
		case OwnQueen:
		return "\033[1;37m\u265B\033[0m";
		case OwnRook:
		return "\033[1;37m\u265C\033[0m";
		case OwnBishop:
		return "\033[1;37m\u265D\033[0m";
		case OwnKnight:
		return "\033[1;37m\u265E\033[0m";
		case OwnPawn:
		return "\033[1;37m\u265F\033[0m";
		default:
		return "x";
	}
}

piece getChessPiece(char input) {
	switch (input) {
		case ' ':
		return None;

		case 'k':
		return EnemyKing;

		case 'q':
		case 'd':
		return EnemyQueen;

		case 'r':
		case 't':
		return EnemyRook;

		case 'b':
		case 'l':
		return EnemyBishop;

		case 'n':
		case 's':
		return EnemyKnight;

		case 'p':
		return EnemyPawn;

		case 'K':
		return OwnKing;

		case 'Q':
		case 'D':
		return OwnQueen;

		case 'R':
		case 'T':
		return OwnRook;

		case 'B':
		case 'L':
		return OwnBishop;

		case 'N':
		case 'S':
		return OwnKnight;

		case 'P':
		return OwnPawn;

		default:
		return None;
	}
}

Board::Board() {
	this->initEmptyField();
}

Board::Board(Board const & previous, const bool revert) {
	if (revert) {
		std::transform(previous.fields.rbegin(), previous.fields.rend(), this->fields.begin(), invertPiece);
		std::reverse_copy(previous.castling.begin(), previous.castling.end(), this->castling.begin());
		for (unsigned char i = 0; i < 8; ++i) {
			this->enPassant[i] = false;
		}
		this->kingPos[0] = 63 - previous.kingPos[1];
		this->kingPos[1] = 63 - previous.kingPos[0];
		this->check[0] = previous.check[1];
		this->check[1] = previous.check[0];
	} else {
		std::copy(previous.fields.begin(), previous.fields.end(), this->fields.begin());
		std::copy(previous.castling.begin(), previous.castling.end(), this->castling.begin());
		for (unsigned char i = 0; i < 8; ++i) {
			this->enPassant[i] = false;
		}
		this->kingPos[0] = previous.kingPos[0];
		this->kingPos[1] = previous.kingPos[1];
		this->check[0] = previous.check[0];
		this->check[1] = previous.check[1];
	}
}

Board::Board(std::string input) {
	if (input.size() != 64) {
		this->initEmptyField();
		return;
	}
	for (uint i = 0; i < 64; ++i) {
		this->fields[i] = getChessPiece(input[i]);
	}

	// Check if castling is still allowed:
	for (auto& i : this->castling) {
		i = false;
	}
	if (this->fields[4] == EnemyKing) {
		if (this->fields[0] == EnemyRook) {
			this->castling[0] = true;
		}
		if (this->fields[7] == EnemyRook) {
			this->castling[1] = true;
		}
	}
	if (this->fields[60] == OwnKing) {
		if (this->fields[56] == OwnRook) {
			this->castling[2] = true;
		}
		if (this->fields[63] == OwnRook) {
			this->castling[3] = true;
		}
	}
	for (unsigned char i = 0; i < 8; ++i) {
		this->enPassant[i] = false;
	}
	for (unsigned char i = 0; i < 2; ++i) {
		this->check[i] = false;
	}
	this->kingPos[0] = this->getNextPiece(OwnKing, 0);
	this->kingPos[1] = this->getNextPiece(EnemyKing, 0);
	this->check[0] = isThreatened(this->kingPos[0]);
	this->check[1] = isThreatened(this->kingPos[1], Own);
}

void Board::initEmptyField() {
	this->fields[0] = EnemyRook;
	this->fields[1] = EnemyKnight;
	this->fields[2] = EnemyBishop;
	this->fields[3] = EnemyQueen;
	this->fields[4] = EnemyKing;
	this->fields[5] = EnemyBishop;
	this->fields[6] = EnemyKnight;
	this->fields[7] = EnemyRook;
	for (unsigned char i = 8; i < 16; ++i) {
		this->fields[i] = EnemyPawn;
	}
	for (unsigned char i = 16; i < 48; ++i) {
		this->fields[i] = None;
	}
	for (unsigned char i = 48; i < 56; ++i) {
		this->fields[i] = OwnPawn;
	}
	this->fields[56] = OwnRook;
	this->fields[57] = OwnKnight;
	this->fields[58] = OwnBishop;
	this->fields[59] = OwnQueen;
	this->fields[60] = OwnKing;
	this->fields[61] = OwnBishop;
	this->fields[62] = OwnKnight;
	this->fields[63] = OwnRook;
	for (unsigned char i = 0; i < 4; ++i) {
		this->castling[i] = true;
	}
	for (unsigned char i = 0; i < 8; ++i) {
		this->enPassant[i] = false;
	}
	this->kingPos[0] = 60;
	this->kingPos[1] = 4;
	this->check[0] = false;
	this->check[1] = false;
}

std::string Board::print() const {
	std::string result;
	//std::string black = "\033[40m";
	//std::string white = "\033[47m";
	for (unsigned char y = 0; y < 8; ++y) {
		result += std::to_string(8-y);
		for (unsigned char x = 0; x < 8; ++x) {
			result += getChessSymbol(this->fields[y*8+x]);
		}
		result += "\n";
	}
	result += " abcdefgh\n";
	return result;
}

std::string Board::display() const {
	std::string result;
	//std::string black = "\033[40m";
	//std::string white = "\033[47m";
	result += "╔═╦═╦═╦═╦═╦═╦═╦═╦═╗\n";
	for (unsigned char y = 0; y < 8; ++y) {
		result += "║" + std::to_string(8-y) + "║";
		for (unsigned char x = 0; x < 8; ++x) {
			result += getChessSymbol(this->fields[y*8+x]);
			result += "║";
		}
		result += "\n╠═╬═╬═╬═╬═╬═╬═╬═╬═╣\n";
	}
	result += "║ ║a║b║c║d║e║f║g║h║\n╚═╩═╩═╩═╩═╩═╩═╩═╩═╝\n";
	return result;
}

// returns whether or not I am in checkmate.
bool Board::checkmate() const { // TODO: needs to be implemented.
	return false;
}

bool Board::isCheck() const {
	return this->check[0];
}

// return whether or not this board is playable.
bool Board::isValid() const { // TODO: needs to be implemented.
	if (std::count_if(this->fields.begin(), this->fields.end(), [](auto x){return x == OwnKing;}) != 1) {
		return false;
	}
	if (std::count_if(this->fields.begin(), this->fields.end(), [](auto x){return x == EnemyKing;}) != 1) {
		return false;
	}
	return true;
}

unsigned char Board::countPieces(const piece type) const {
	unsigned char result = 0;
	for (unsigned char i = 0; i < 64; ++i) {
		if (this->fields[i] == type) {
			++result;
		}
	}
	return result;
}
/*
void Board::getNextPiece(const piece type, unsigned char *pos) const {
	for (++(*pos); *pos < 64; ++(*pos)) {
		if (this->fields[*pos] == type) {
			return;
		}
	}
}
*/
unsigned char Board::getNextPiece(const piece type, unsigned char pos) const {
	for (++pos; pos < 64; ++pos) {
		if (this->fields[pos] == type) {
			return pos;
		}
	}
	return 64;
}

std::vector<Move> Board::getValidMoves() const {
	std::vector<Move> result;
	for (uint i = 0; i < 64; ++i) {
		std::vector<Move> tmp = this->getValidMoves(i);
		result.insert(std::end(result), std::begin(tmp), std::end(tmp));
	}
	return result;
}

std::vector<Move> Board::getValidMoves(const unsigned char pos) const {
	auto addPawnMoves = [](std::vector<Move>& result, const unsigned char from, const unsigned char to) {
		if (from < 8) {
			result.emplace_back(from, from, OwnQueen);
			result.emplace_back(from, from, OwnRook);
			result.emplace_back(from, from, OwnBishop);
			result.emplace_back(from, from, OwnKnight);
		} else if (from < 16) {
			result.emplace_back(from, to, OwnQueen);
			result.emplace_back(from, to, OwnRook);
			result.emplace_back(from, to, OwnBishop);
			result.emplace_back(from, to, OwnKnight);
		} else {
			result.emplace_back(from, to);
		}
	};
	std::vector<Move> result;
	if (this->fields[pos] & Own) {
		if (this->fields[pos] == OwnPawn) {
			if (this->fields[pos-8] == None) {
				addPawnMoves(result, pos, pos-8);
			}
			if (pos > 47 && pos < 56 && this->fields[pos-8] == None && this->fields[pos-16] == None) {
				addPawnMoves(result, pos, pos-16);
			}
			if (pos % 8 != 7 && this->fields[pos-7] & Enemy) {
				addPawnMoves(result, pos, pos-7);
			}
			if (pos % 8 != 0 && this->fields[pos-9] & Enemy) {
				addPawnMoves(result, pos, pos-9);
			}
		} else if (this->fields[pos] == OwnKnight) {
			std::array<char, 8> movingTo = {-17, -15, -10, -6, 6, 10, 15, 17};
			if (pos / 8 < 2) {
				movingTo[0] = 0; // -17
				movingTo[1] = 0; // -15
				if (pos / 8 < 1) {
					movingTo[2] = 0; // -10
					movingTo[3] = 0; // -6
				}
			}
			if (pos / 8 > 5) {
				movingTo[7] = 0; // 17
				movingTo[6] = 0; // 15
				if (pos / 8 > 6) {
					movingTo[5] = 0; // 10
					movingTo[4] = 0; // 6
				}
			}
			if (pos % 8 < 2) {
				movingTo[2] = 0; // -10
				movingTo[4] = 0; // 6
				if (pos % 8 < 1) {
					movingTo[0] = 0; // -17
					movingTo[6] = 0; // 15
				}
			}
			if (pos % 8 > 5) {
				movingTo[3] = 0; // -6
				movingTo[5] = 0; // 10
				if (pos % 8 > 6) {
					movingTo[1] = 0; // -15
					movingTo[7] = 0; // 17
				}
			}
			for (auto i : movingTo) {
				if (i && !(this->fields[pos + i] & Own)) {
					result.emplace_back(pos, pos + i);
				}
			}
		} else if (this->fields[pos] == OwnKing) {
			std::array<char, 14> movingTo = {-9, -8, -7, -1, 1, 7, 8, 9, 0, 0, 0, 0, 0, 0};
			if (pos / 8 == 0) {
				movingTo[0] = 0; // -9
				movingTo[1] = 0; // -8
				movingTo[2] = 0; // -7
			}
			if (pos / 8 == 7) {
				movingTo[7] = 0; // 9
				movingTo[6] = 0; // 8
				movingTo[5] = 0; // 7
			}
			if (pos % 8 == 0) {
				movingTo[0] = 0; // -9
				movingTo[3] = 0; // -1
				movingTo[5] = 0; // 7
			}
			if (pos % 8 == 7) {
				movingTo[2] = 0; // -7
				movingTo[4] = 0; // 1
				movingTo[7] = 0; // 9
			}
			// TODO: the following castling code looks ugly and some of it redundand
			if (!this->isThreatened(pos)) {
				if (this->castling[0] == true && this->fields[61] == None && this->fields[62] == None && this->fields[63] == Rook && !this->isThreatened(62) && !this->isThreatened(61)) {
					if (pos == 60) {
						movingTo[8] = 2;
					} else if (pos == 59 && this->fields[60] == None) {
						movingTo[9] = 3;
					}
				}
				if (this->castling[1] == true && this->fields[58] == None && this->fields[57] == None && this->fields[56] == Rook && !this->isThreatened(57)) {
					if (pos == 59) {
						movingTo[10] = -2;
					} else if (pos == 60 && this->fields[59] == None) {
						movingTo[11] = -3;
					}

				}
			}
			if (this->castling[0] == true && !this->isThreatened(pos) && (this->fields[60] == None || this->fields[60] == OwnKing) && this->fields[61] == None && this->fields[62] == None && this->fields[63] == OwnRook) {
				movingTo[12] = 2 + this->fields[60] == None;
			}
			if (this->castling[1] == true && (this->fields[59] == None || this->fields[59] == OwnKing) && this->fields[58] == None && this->fields[57] == None && this->fields[56] == OwnRook) {
				movingTo[13] = -3 + this->fields[60] == OwnKing;
			}
			for (auto i : movingTo) {
				if (i && !(this->fields[pos + i] & Own)) {
					result.emplace_back(pos, pos + i);
				}
			}
		} else {
			std::array<char, 8> directions = {0, 0, 0, 0, 0, 0, 0, 0};
			if ((this->fields[pos] & OwnBishop) == OwnBishop) {
				directions[0] = -9;
				directions[1] = -7;
				directions[2] = 7;
				directions[3] = 9;
			}
			if ((this->fields[pos] & OwnRook) == OwnRook) {
				directions[4] = -8;
				directions[5] = -1;
				directions[6] = 1;
				directions[7] = 8;
			}
			for (auto direction : directions) {
				if (direction) {
					for (unsigned char i = 1; i < getDirectionSize(pos, direction); ++i) {
						if (this->fields[pos + i * direction] & Own) {
							break;
						} else {
							result.emplace_back(pos, pos + i * direction);
							if (this->fields[pos + i * direction] & Enemy) {
								break;
							}
						}
					}
				}
			}
		}
	}
	return result;
}

bool Board::isValidMove(const unsigned char from, const unsigned char to) const {
	char x1 = from % 8;
	char y1 = from / 8;
	char x2 = to % 8;
	char y2 = to / 8;
	if (from >= 64) {
		std::cout << "from: " << from << "\n";
		std::cout << "to: " << to << "\n";
	}
	if (to >= 64) {
		std::cout << "from: " << from << "\n";
		std::cout << "to: " << to << "\n";
	}
	// std::cout << "checking validity for " << (int) from << "-" << (int) to << " aka (" << (int) x1 << "-" << (int) y1 << ")-(" << (int) x2 << "-" << (int) y2 << ") with " << this->fields[from] << "-" << this->fields[to] << "\n";
	if (from == to) {
		std::cerr << "Can't move to the field you came from.\n";
		return false;
	} else if (this->fields[from] == None) {
		std::cerr << "Can't move from an empty field.\n";
		return false;
	} else if (this->fields[from] & Enemy) {
		std::cerr << "Can't move an enemy figure.\n";
		return false;
	} else if (this->fields[to] & Own) {
		std::cerr << "Can't move to a field occupied by an own figure.\n";
		return false;
	} else if (this->fields[from] == OwnPawn) {
		if (y1 == y2 + 1) {
			if (x1 == x2 && this->fields[to] == None) {
				return true;
			} else if (x1 + 1 == x2 && (this->fields[to] & Enemy)) {
				return true;
			} else if (x1 - 1 == x2 && (this->fields[to] & Enemy)) {
				return true;
			} else {
				std::cerr << "This is not a valid move for a pawn (x).\n";
				return false;
			}
		} else if (y1 == 6 && y2 == 4 && this->fields[to] == None && this->fields[(from + to) / 2] == None) {
				return true;
		} else {
			std::cerr << "This is not a valid move for a pawn (y).\n";
			return false;
		}
	} else if (this->fields[from] == OwnKnight) {
		if (y1 + 1 == y2 || y1 == y2 + 1) {
			if (x1 + 2 == x2 || x1 == x2 + 2) {
				return true;
			} else {
				std::cerr << "This is not a valid move for a knight.\n";
				return false;
			}
		} else if (y1 + 2 == y2 || y1 == y2 + 2) {
			if (x1 + 1 == x2 || x1 == x2 + 1) {
				return true;
			} else {
				std::cerr << "This is not a valid move for a knight.\n";
				return false;
			}
		} else {
			return false;
		}
	} else if (this->fields[from] == OwnKing) {
		if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2) {
			return true;
		} else if (this->castling[0] && this->isCastlingAllowed(0)) {
			return true;
		} else if (this->castling[1] && this->isCastlingAllowed(1)) {
			return true;
		} else {
			std::cerr << "This is not a valid move for a king.\n";
			return false;
		}
	} else {
		if ((this->fields[from] & OwnBishop) == OwnBishop) { // bishop or queen - can't return something here in some cases because of queen
			if (x1 + y1 == x2 + y2) {
				int xstart = std::min(x1, x2) + 1;
				int xend = std::max(x1, x2) - 1;
				int ystart = std::max(y1, y2) - 1;
				for (int i = 0; i < xend - xstart; ++i) {
					if (this->fields[(ystart - i) * 8 + i + xstart] != None) {
						std::cerr << "There is a figure in the way at " << (char) ('a' + ((ystart - i) * 8 + i + xstart) % 8) << 8 - ((ystart - i) * 8 + i + xstart) / 8 << ".\n";
						return false;
					}
				}
				return true;
			} else if (x1 - y1 == x2 - y2) {
				int xstart = std::min(x1, x2) + 1;
				int xend = std::max(x1, x2) - 1;
				int ystart = std::min(y1, y2) + 1;
				for (int i = 0; i < xend - xstart; ++i) {
					if (this->fields[(ystart + i) * 8 + i + xstart] != None) {
						std::cerr << "There is a figure in the way at " << (char) ('a' + ((ystart + i) * 8 + i + xstart) % 8) << 8 - ((ystart + i) * 8 + i + xstart) / 8 << ".\n";
						return false;
					}
				}
				return true;
			} // no else here on purpose - might be a queen which is allowed other stuff
		}
		if ((this->fields[from] & OwnRook) == OwnRook) { // rook or queen - can return something here in all cases because it was not a possible bishop move
			if (x1 == x2) {
				for (unsigned char i = std::min(from, to) + 8; i < std::max(from, to) - 8; i += 8) {
					if (this->fields[i] != None) {
						return false;
					}
				}
				return true;
			} else if (y1 == y2) {
				for (unsigned char i = std::min(from, to) + 1; i < std::max(from, to) - 1; ++i) {
					if (this->fields[i] != None) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}

bool Board::isValidMove(const Move& move) const {
	return this->isValidMove(move.from, move.to);
}

// Checks if a move is valid. Takes algebraic notation as input (see xboard documentation for more details).
bool Board::isValidMove(const std::string& move) const {
	if (move.length() >= 4) { // we don't care about pawn promotions when checking for validity.
		unsigned char from_x = ((move[0] & 15) - 1) & 7;
		unsigned char from_y = 7 - (((move[1] & 15) - 1) & 7);
		unsigned char to_x = ((move[2] & 15) - 1) & 7;
		unsigned char to_y = 7 - (((move[3] & 15) - 1) & 7);
		// std::cout << "checking " << this->fields[from_y * 8 + from_x] << " to " << this->fields[to_y * 8 + to_x] << "\n";
		return this->isValidMove(from_y * 8 + from_x, to_y * 8 + to_x);
	} else {
		return false;
	}
}

bool Board::isThreatened(const unsigned char pos, const piece opponent) const {
	for (char direction : {-8, -1, 1, 8}) {
		for (unsigned char i = 1; i < getDirectionSize(pos, direction); ++i) {
			if ((this->fields[pos + i * direction] & (opponent | Rook)) == (opponent | Rook)) {
				return true;
			} else if (this->fields[pos + i * direction] != None) {
				break;
			}
		}
		if (getDirectionSize(pos, direction) > 0 && this->fields[pos + direction] == (opponent | King)) {
			return true;
		}
	}
	for (char direction : {-9, -7, 7, 9}) {
		for (unsigned char i = 1; i < getDirectionSize(pos, direction); ++i) {
			if ((this->fields[pos + i * direction] & (opponent | Bishop)) == (opponent | Bishop)) {
				return true;
			} else if (this->fields[pos + i * direction] != None) {
				break;
			}
		}
		if (getDirectionSize(pos, direction) > 0 && this->fields[pos + direction] == (opponent | King)) {
			return true;
		}
	}

	// Knight
	std::array<char, 8> movingFrom = {-17, -15, -10, -6, 6, 10, 15, 17};
	if (pos / 8 < 2) {
		movingFrom[0] = 0; // -17
		movingFrom[1] = 0; // -15
		if (pos / 8 < 1) {
			movingFrom[2] = 0; // -10
			movingFrom[3] = 0; // -6
		}
	}
	if (pos / 8 > 5) {
		movingFrom[7] = 0; // 17
		movingFrom[6] = 0; // 15
		if (pos / 8 > 6) {
			movingFrom[5] = 0; // 10
			movingFrom[4] = 0; // 6
		}
	}
	if (pos % 8 < 2) {
		movingFrom[2] = 0; // -10
		movingFrom[4] = 0; // 6
		if (pos % 8 < 1) {
			movingFrom[0] = 0; // -17
			movingFrom[6] = 0; // 15
		}
	}
	if (pos % 8 > 5) {
		movingFrom[3] = 0; // -6
		movingFrom[5] = 0; // 10
		if (pos % 8 > 6) {
			movingFrom[1] = 0; // -15
			movingFrom[7] = 0; // 17
		}
	}
	for (auto i : movingFrom) {
		if (i && this->fields[pos + i] == (opponent | Knight)) {
			return true;
		}
	}

	// Pawn
	if (pos > 7) {
		if ((pos & 7) != 0) {
			if (this->fields[pos - 9] == (opponent | Pawn)) {
				return true;
			}
		}
		if ((pos & 7) != 7) {
			if (this->fields[pos - 7] == (opponent | Pawn)) {
				return true;
			}
		}
	}

	return false;
}

void Board::updateThreats(const unsigned char from, const unsigned char to) {
	for (std::size_t i = 0; i < 2; ++i) {
		if (this->kingPos[i] == to) {
			this->check[i] = this->isThreatened(this->kingPos[i], Enemy);
		} else {
			this->check[i] = this->check[i] || this->threatens(to, kingPos[i]);
			this->check[i] = this->check[i] || this->opensThreat(from, kingPos[i]);
		}
	}
}

bool Board::threatens(const unsigned char from, const unsigned char to) const {
	char x1 = from % 8;
	char y1 = from / 8;
	char x2 = to % 8;
	char y2 = to / 8;
	piece player;
	piece opponent;
	if (this->fields[from] & Enemy) {
		player = Enemy;
		opponent = Own;
	} else {
		player = Own;
		opponent = Enemy;
	}
	if (this->fields[from] == None) {
		return false;
	} else if (this->fields[from] == (player | Pawn)) {
		if (y1 == y2 + 1) {
			if (x1 == x2 && this->fields[to] == None) {
				return true;
			} else if (x1 + 1 == x2 && (this->fields[to] & opponent)) {
				return true;
			} else if (x1 - 1 == x2 && (this->fields[to] & opponent)) {
				return true;
			} else {
				return false;
			}
		} else if (y1 == 6 && y2 == 4 && this->fields[to] == None && this->fields[(from + to) / 2] == None) {
				return true;
		} else {
			return false;
		}
	} else if (this->fields[from] == (player | Knight)) {
		if (y1 + 1 == y2 || y1 == y2 + 1) {
			if (x1 + 2 == x2 || x1 == x2 + 2) {
				return true;
			} else {
				return false;
			}
		} else if (y1 + 2 == y2 || y1 == y2 + 2) {
			if (x1 + 1 == x2 || x1 == x2 + 1) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else if (this->fields[from] == (player | King)) {
		if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2) {
			return true;
		} else if (this->castling[0] && this->isCastlingAllowed(0)) {
			return true;
		} else if (this->castling[1] && this->isCastlingAllowed(1)) {
			return true;
		} else {
			return false;
		}
	} else {
		if ((this->fields[from] & OwnBishop) == (player | Bishop)) { // bishop or queen - can't return something here in some cases because of queen
			if (x1 + y1 == x2 + y2) {
				int xstart = std::min(x1, x2) + 1;
				int xend = std::max(x1, x2) - 1;
				int ystart = std::max(y1, y2) - 1;
				for (int i = 0; i < xend - xstart; ++i) {
					if (this->fields[(ystart - i) * 8 + i + xstart] != None) {
						return false;
					}
				}
				return true;
			} else if (x1 - y1 == x2 - y2) {
				int xstart = std::min(x1, x2) + 1;
				int xend = std::max(x1, x2) - 1;
				int ystart = std::min(y1, y2) + 1;
				for (int i = 0; i < xend - xstart; ++i) {
					if (this->fields[(ystart + i) * 8 + i + xstart] != None) {
						return false;
					}
				}
				return true;
			} // no else here on purpose - might be a queen which is allowed other stuff
		}
		if ((this->fields[from] & OwnRook) == (player | Rook)) { // rook or queen - can return something here in all cases because it was not a possible bishop move
			if (x1 == x2) {
				for (unsigned char i = std::min(from, to) + 8; i < std::max(from, to) - 8; i += 8) {
					if (this->fields[i] != None) {
						return false;
					}
				}
				return true;
			} else if (y1 == y2) {
				for (unsigned char i = std::min(from, to) + 1; i < std::max(from, to) - 1; ++i) {
					if (this->fields[i] != None) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}

// TODO: needs review and testing. I don't trust this solution.
bool Board::opensThreat(const unsigned char from, const unsigned char to) const {
	piece opponent;
	if (this->fields[to] & Enemy) {
		opponent = Own;
	} else {
		opponent = Enemy;
	}
	if (from == to) {
		return false;
	}
	char direction = (char) from - to;
	while (std::abs(direction) > 9 && std::abs(direction) % 2 == 0) {
		direction /= 2;
	}
	if (std::abs(direction) > 9 && direction % 2 == 1) {
		return false;
	}
	for (char i = 1; i < getDirectionSize(from, direction); ++i) {
		if (this->fields[to + i * direction] & opponent) {
			return this->threatens(to + i * direction, to);
		}
	}
	return false;
}

std::vector<char> Board::getThreatRelevantPositions(const unsigned char pos) const {

	std::vector<char> result;
	result.reserve(20);

	for (char direction : {-8, -1, 1, 8}) {
		for (unsigned char i = 1; i < getDirectionSize(pos, direction); ++i) {
			if ((this->fields[pos + i * direction] & EnemyRook) == EnemyRook) {
				result.push_back(pos + i * direction);
				break;
			} else if (this->fields[pos + i * direction] != None) {
				break;
			} else {
				result.push_back(pos + i * direction);
			}
		}
	}
	for (char direction : {-9, -7, 7, 9}) {
		for (unsigned char i = 1; i < getDirectionSize(pos, direction); ++i) {
			if ((this->fields[pos + i * direction] & EnemyBishop) == EnemyBishop) {
				result.push_back(pos + i * direction);
				break;
			} else if (this->fields[pos + i * direction] != None) {
				break;
			} else {
				result.push_back(pos + i * direction);
			}
		}
	}

	// Knight
	std::array<char, 8> movingFrom = {-17, -15, -10, -6, 6, 10, 15, 17};
	if (pos / 8 < 2) {
		movingFrom[0] = 0; // -17
		movingFrom[1] = 0; // -15
		if (pos / 8 < 1) {
			movingFrom[2] = 0; // -10
			movingFrom[3] = 0; // -6
		}
	}
	if (pos / 8 > 5) {
		movingFrom[7] = 0; // 17
		movingFrom[6] = 0; // 15
		if (pos / 8 > 6) {
			movingFrom[5] = 0; // 10
			movingFrom[4] = 0; // 6
		}
	}
	if (pos % 8 < 2) {
		movingFrom[2] = 0; // -10
		movingFrom[4] = 0; // 6
		if (pos % 8 < 1) {
			movingFrom[0] = 0; // -17
			movingFrom[6] = 0; // 15
		}
	}
	if (pos % 8 > 5) {
		movingFrom[3] = 0; // -6
		movingFrom[5] = 0; // 10
		if (pos % 8 > 6) {
			movingFrom[1] = 0; // -15
			movingFrom[7] = 0; // 17
		}
	}
	for (auto i : movingFrom) {
		if (i) {
			result.push_back(pos + i);
		}
	}

	return result;
}

bool Board::isCastlingAllowed(const bool which) const {
	if (!this->castling[which]) {
		return false;
	}
	if (this->fields[63 - which * 7] != OwnRook) {
		return false;
	}
	if (this->fields[60] == OwnKing) {
		return !this->isThreatened(60) && !this->isThreatened(59 + 2 * which) && !this->isThreatened(58 + 4 * which);
	} else if (this->fields[59] == OwnKing) {
		return !this->isThreatened(59) && !this->isThreatened(58 + 2 * which) && !this->isThreatened(57 + 4 * which);
	} else {
		return false;
	}
}

// This function does intentionally __not__ check wether the given move is valid.
Board Board::applyMove(const unsigned char from, const unsigned char to, const piece turn_to, bool rotateBoard) const {
	Board result = Board(*this, rotateBoard);
	// disable castling if no longer allowed:
	if (from == 63) {
		result.castling[3] = false;
	}
	if (from == 56) {
		result.castling[2] = false;
	}
	if (this->fields[from] == OwnKing) {
		result.castling[3] = false;
		result.castling[2] = false;
	}
	if (rotateBoard) {
		result.fields[63 - to] = result.fields[63 - from];
		result.fields[63 - from] = None;

		// execute castling:
		if (this->fields[from] == OwnKing && (abs(from - to) == 2 || abs(from - to) == 3)) {
			if (to > from) {
				result.fields[62 - to] = result.fields[0];
				result.fields[0] = None;
			}
			if (to < from) {
				result.fields[64 - to] = result.fields[7];
				result.fields[7] = None;
			}
		}

		// execute pawn promotion:
		if (to < 8 && result.fields[63 - to] == EnemyPawn && turn_to != None) {
			result.fields[63 - to] = invertPiece(turn_to);
		}

		// save en passant possibilities
		if (from > 47 && from < 56 && to == from - 16) {
			result.enPassant[55 - from] = true;
		}

		// update king position:
		if (this->fields[from] == OwnKing) {
			result.kingPos[1] = 63 - to;
			result.updateThreats(63 - from, 63 - to);
		}
	} else {
		result.fields[to] = result.fields[from];
		result.fields[from] = None;

		// execute castling:
		if (this->fields[from] == OwnKing && (abs(from - to) == 2 || abs(from - to) == 3)) {
			if (to > from) {
				result.fields[to] = result.fields[63];
				result.fields[63] = None;
			}
			if (to < from) {
				result.fields[to] = result.fields[56];
				result.fields[56] = None;
			}
		}

		// execute pawn promotion:
		if (to < 8 && result.fields[to] == OwnPawn && turn_to != None) {
			result.fields[to] = turn_to;
		}

		// update king position:
		if (this->fields[from] == OwnKing) {
			result.kingPos[0] = to;
			result.updateThreats(from, to);
		}
	}
	return result;
}

Board Board::applyMove(const Move move, bool rotateBoard) const {
	return this->applyMove(move.from, move.to, move.turn_to, rotateBoard);
}

Board Board::applyMove(const std::string move, bool rotateBoard) const {
	// TODO: castling
	if (move.length() >= 4) {
		unsigned char from_x = ((move[0] & 15) - 1) & 7;
		unsigned char from_y = 7 - (((move[1] & 15) - 1) & 7);
		unsigned char to_x = ((move[2] & 15) - 1) & 7;
		unsigned char to_y = 7 - (((move[3] & 15) - 1) & 7);
		// std::cout << "moving " << this->fields[from_y * 8 + from_x] << " to " << this->fields[to_y * 8 + to_x] << "\n";
		piece turn_to;
		if (move.length() > 4) {
			turn_to = getPieceFromNotation(move[4]);
		} else {
			turn_to = None;
		}
		return this->applyMove(from_y * 8 + from_x, to_y * 8 + to_x, turn_to, rotateBoard);
	} else {
		return Board();
	}
}

std::vector<unsigned char> getBetweenFields(unsigned char from, unsigned char to) {
	//char min = std::min(from, to);
	//char max = std::max(from, to);
	std::vector<unsigned char> result;
	if (from == to) {
		// no need to do the calculations.
	} else if (from / 8 == to / 8) {
		for (unsigned char i = std::min(from, to) + 1; i < std::max(from, to); ++i) {
			result.push_back(i);
		}
	} else if (from % 8 == to % 8) {
		for (unsigned char i = std::min(from, to) + 8; i < std::max(from, to); i += 8) {
			result.push_back(i);
		}
	} else if (from / 8 + from % 8 == to / 8 + to % 8) {
		for (unsigned char i = std::min(from, to) + 7; i < std::max(from, to); i += 7) {
			result.push_back(i);
		}
	} else if (from / 8 - from % 8 == to / 8 - to % 8) {
		for (unsigned char i = std::min(from, to) + 9; i < std::max(from, to); i += 9) {
			result.push_back(i);
		}
	}
	return result;
}

bool Board::operator==(const Board& other) const {
	for (unsigned char i = 0; i < 64; ++i) {
		if (this->fields[i] != other.fields[i]) {
			return false;
		}
	}
	for (unsigned char i = 0; i < 4; ++i) {
		if (this->castling[i] != other.castling[i]) {
			return false;
		}
	}
	return true;
}

piece operator|(const piece a, const piece b) {
	return static_cast<piece>(static_cast<int>(a) | static_cast<int>(b));
}

piece operator&(const piece a, const piece b) {
	return static_cast<piece>(static_cast<int>(a) & static_cast<int>(b));
}

piece operator^(const piece a, const piece b) {
	return static_cast<piece>(static_cast<int>(a) ^ static_cast<int>(b));
}

piece invertPiece(const piece input) {
	return input == None ? None : input ^ (Own | Enemy);
}

piece getPieceFromNotation(const char input) {
	piece result = None;
	if (input == 'N' || input == 'n' || input == 'S' || input == 's') {
		result = Knight;
	} else if (input == 'B' || input == 'b') {
		result = Bishop;
	} else if (input == 'R' || input == 'r') {
		result = Rook;
	} else if (input == 'Q' || input == 'q') {
		result = Queen;
	} else if (input == 'K' || input == 'k') {
		result = King;
	}
	return result;
}

std::ostream& operator<<(std::ostream& stream, const piece& obj) {
	stream << getChessSymbol(obj);
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const Board& board){
	for (uint i = 0; i < 8; ++i) {
		stream << 8 - i;
		for (uint j = 0; j < 8; ++j) {
			if ((i + j) & 1) {
				stream << "\033[0m";
			} else {
				stream << "\033[48;5;0m";
			}
			stream << getChessSymbol(board.fields[i*8+j]);
		}
		if (i & 1) {
			stream << "\033[0m";
		}
		stream << "\n";
	}
	stream << " abcdefgh\n";
	return stream;
}

bool operator<(const Board& board1, const Board& board2) {
	return board1.fields < board2.fields || (board1.fields == board2.fields && board1.castling < board2.castling);
}
/*
bool operator==(const Board& board1, const Board& board2) {
	return board1.fields == board2.fields && board1.castling == board2.castling && board1.enPassant == board2.enPassant && board1.check == board2.check && board1.kingPos == board2.kingPos;
}
*/
constexpr unsigned char getDirectionSize(const unsigned char pos, const char direction) {
	switch (direction) {
		case -8:
		return pos / 8;
		case 8:
		return 7 - pos / 8;
		case -1:
		return pos & 7;
		case 1:
		return 7 - (pos & 7);
		case -9:
		return std::min(pos / 8, pos & 7);
		case -7:
		return std::min(pos / 8, 7 - (pos & 7));
		case 7:
		return std::min(7 - pos / 8, pos & 7);
		case 9:
		return std::min(7 - pos / 8, 7 - (pos & 7));
		default:
		return 0;
	}
	return 0;
}
