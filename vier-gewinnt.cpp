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

	template<std::uint8_t max_depth>
	std::uint64_t botMove();

	friend bool operator==(Field f1, Field f2);
	friend bool operator!=(Field f1, Field f2);
};

Field reverse(Field input) {
	Field output{input};
	output.reverse();
	return output;
}

struct ScoredMove {
	Field field;
	std::uint8_t currentMove;
	std::uint8_t bestMove;
	std::uint8_t score;
};

template<std::uint8_t depth>
ScoredMove getBestMove(Field field) {
	if constexpr (depth == 0) { // either use even numbers for the depth or change the "default" score.
		return ScoredMove{field, 0, 0, static_cast<std::uint8_t>(-64)};
	} else {
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
				std::uint8_t currentScore = -getBestMove<depth - 1>(result.field).score;
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
}

template<std::uint8_t max_depth>
std::uint64_t Field::botMove() {
	return play(getBestMove<max_depth>(::reverse(*this)).bestMove);
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

int main(int argc, char** argv) {
	Field field;
	int input;
	std::uint64_t currentPlayerMove;
	std::uint64_t currentBotMove;

	int difficulty;
	std::cout << "Choose a difficulty between 1 and 23 (perfect opponent). Note that the opponent gets slower with each difficulty. The default difficulty is 4.\n";
	std::cin >> difficulty;
	if (difficulty < 1 || difficulty > 23) {
		std::cout << "Difficulty not between 1 and 23, using the default (4).\n";
	}
	do {
		field.reverse();
		std::cout << field.print();
		std::cin >> input;
		while ((input < 1 || input > 7 || !(currentPlayerMove = field.play(1 << (input - 1)))) && __builtin_popcountll(field.player1 | field.player2) < 49) {
			std::cin >> input;
		}
		field.reverse();
		switch (difficulty) {
			case 1: currentBotMove = field.botMove<4>(); break;
			case 2: currentBotMove = field.botMove<6>(); break;
			case 3: currentBotMove = field.botMove<8>(); break;
			case 4: currentBotMove = field.botMove<10>(); break;
			case 5: currentBotMove = field.botMove<12>(); break;
			case 6: currentBotMove = field.botMove<14>(); break;
			case 7: currentBotMove = field.botMove<16>(); break;
			case 8: currentBotMove = field.botMove<18>(); break;
			case 9: currentBotMove = field.botMove<20>(); break;
			case 10: currentBotMove = field.botMove<22>(); break;
			case 11: currentBotMove = field.botMove<24>(); break;
			case 12: currentBotMove = field.botMove<26>(); break;
			case 13: currentBotMove = field.botMove<28>(); break;
			case 14: currentBotMove = field.botMove<30>(); break;
			case 15: currentBotMove = field.botMove<32>(); break;
			case 16: currentBotMove = field.botMove<34>(); break;
			case 17: currentBotMove = field.botMove<36>(); break;
			case 18: currentBotMove = field.botMove<38>(); break;
			case 19: currentBotMove = field.botMove<40>(); break;
			case 20: currentBotMove = field.botMove<42>(); break;
			case 21: currentBotMove = field.botMove<44>(); break;
			case 22: currentBotMove = field.botMove<46>(); break;
			case 23: currentBotMove = field.botMove<48>(); break;
			default: currentBotMove = field.botMove<10>(); break;
		}
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