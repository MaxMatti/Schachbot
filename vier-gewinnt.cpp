#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#define MAXINT std::numeric_limits<unsigned long int>::max()
bool playerWins(const std::uint64_t& player, const std::uint64_t& lastMove);

struct Field {
	std::uint64_t player1 = 0;
	std::uint64_t player2 = 0;

	Field() {}
	void reverse() {
		auto tmp = player1;
		player1 = player2;
		player2 = tmp;
	}

	std::uint64_t play(std::uint8_t move) {
		assert(__builtin_popcount(move) == 1);
		assert(move < (1 << 7));
		std::uint64_t offsetMove = move;
		for (auto i = 0; i < 6 && (offsetMove & player1) == 0 && (offsetMove & player2) == 0; ++i) {
			offsetMove <<= 7;
		}
		offsetMove >>= 7;
		player1 |= offsetMove;
		assert((player1 & player2) == 0);
		return offsetMove;
	}

	std::string print() {
		std::ostringstream tmp;
		tmp << "║1234567║\n";
		for (auto i = 0; i < 6; ++i) {
			tmp << "║";
			for (auto j = 0; j < 7; ++j) {
				if (player1 & (1ull << (i * 7 + j))) {
					if (player2 & (1ull << (i * 7 + j))) {
						tmp << "E";
					} else {
						tmp << "X";
					}
				} else if (player2 & (1ull << (i * 7 + j))) {
					tmp << "O";
				} else {
					tmp << " ";
				}
			}
			tmp << "║\n";
		}
		tmp << "╚═══════╝\n";
		return tmp.str();
	}

	std::uint64_t botMove();

	friend bool operator==(Field f1, Field f2);
	friend bool operator!=(Field f1, Field f2);
};

struct ScoredMove {
	Field field;
	std::uint8_t currentMove;
	std::uint8_t bestMove;
	std::uint8_t score;
};

ScoredMove bestMove(Field field, std::uint8_t depth = 0) {
	if (depth > 10) {
		return ScoredMove{field, 0, 0, static_cast<std::uint8_t>(-1)};
	}
	field.reverse();
	ScoredMove result;
	result.field = field;
	result.score = 0;
	for (result.currentMove = 1; result.currentMove < (1 << 7); result.currentMove <<= 1) {
		std::uint64_t currentPos = result.field.play(result.currentMove);
		if (currentPos) {
			if (playerWins(result.field.player1, currentPos)) {
				result.bestMove = result.currentMove;
				result.score = static_cast<std::uint8_t>(-1);
				return result;
			}
			std::uint8_t currentScore = -bestMove(result.field, depth + 1).score;
			if (currentScore < 128) {
				++currentScore;
			} else {
				--currentScore;
			}
			if (currentScore > result.score) {
				result.bestMove = result.currentMove;
				result.score = currentScore;
			}
		}
		result.field = field;
	}
	return result;
}

std::uint64_t Field::botMove() {
	return play(bestMove(*this).bestMove);
}

bool operator==(Field f1, Field f2) {
	return f1.player1 == f2.player1 && f1.player2 == f2.player2;
}

bool operator!=(Field f1, Field f2) {
	return f1.player1 != f2.player1 || f1.player2 != f2.player2;
}

bool playerWins(const std::uint64_t& player, const std::uint64_t& lastMove) {
	const auto offset = __builtin_ctzll(lastMove) % 7;
	// vertical
	if (player & (lastMove << 7)) {
		if (player & (lastMove << 14)) {
			if (player & (lastMove << 21) || player & (lastMove >> 7)) {
				return true;
			}
		} else if (player & (lastMove >> 7) && player & (lastMove >> 14)) {
			return true;
		}
	} else if (player & (lastMove >> 7) && player & (lastMove >> 14) && player & (lastMove >> 21)) {
		return true;
	}
	// horizontal
	if (offset < 7 && player & (lastMove << 1)) {
		if (offset < 6 && player & (lastMove << 2)) {
			if ((offset < 5 && player & (lastMove << 3)) || (offset > 0 && player & (lastMove >> 1))) {
				return true;
			}
		} else if (offset > 1 && player & (lastMove >> 1) && player & (lastMove >> 2)) {
			return true;
		}
	} else if (offset > 2 && player & (lastMove >> 1) && player & (lastMove >> 2) && player & (lastMove >> 3)) {
		return true;
	}
	// diagonal
	if (offset < 7 && player & (lastMove << 8)) {
		if (offset < 6 && player & (lastMove << 16)) {
			if ((offset < 5 && player & (lastMove << 24)) || (offset > 0 && player & (lastMove >> 8))) {
				return true;
			}
		} else if (offset > 1 && player & (lastMove >> 8) && player & (lastMove >> 16)) {
			return true;
		}
	} else if (offset > 2 && player & (lastMove >> 8) && player & (lastMove >> 16) && player & (lastMove >> 24)) {
		return true;
	}
	if (offset < 7 && player & (lastMove >> 6)) {
		if (offset < 6 && player & (lastMove >> 12)) {
			if ((offset < 5 && player & (lastMove >> 18)) || (offset > 0 && player & (lastMove << 6))) {
				return true;
			}
		} else if (offset > 1 && player & (lastMove << 6) && player & (lastMove << 12)) {
			return true;
		}
	} else if (offset > 2 && player & (lastMove << 6) && player & (lastMove << 12) && player & (lastMove << 18)) {
		return true;
	}
	return false;
}

int main() {
	Field field;
	int input;
	std::uint64_t currentPlayerMove;
	std::uint64_t currentBotMove;
	do {
		field.reverse();
		std::cout << field.print();
		std::cin >> input;
		while ((input < 1 || input > 7 || !(currentPlayerMove = field.play(1 << (input - 1)))) && __builtin_popcountll(field.player1 | field.player2) < 49) {
			std::cin >> input;
		}
		field.reverse();
		currentBotMove = field.botMove();
		if (!currentBotMove) {
			std::cerr << "Internal error.\n";
			return 0;
		}
	} while (!playerWins(field.player1, currentBotMove) && !playerWins(field.player2, currentPlayerMove) && __builtin_popcountll(field.player1 | field.player2) < 49);
	field.reverse();
	if (playerWins(field.player1, currentPlayerMove)) {
		std::cout << "Congratulations! You won!\n";
	} else if (playerWins(field.player2, currentBotMove)) {
		std::cout << "Sorry, you lost.\n";
	} else {
		std::cout << "It seems to be a draw.\n";
	}
	std::cout << field.print();
	return 0;
}