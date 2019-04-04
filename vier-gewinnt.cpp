#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <tuple>
#include <vector>

#define MAXINT std::numeric_limits<unsigned long int>::max()
bool playerWins(const std::uint64_t& player);

struct Field {
	std::uint64_t player1 = 0;
	std::uint64_t player2 = 0;

	Field() {}
	void reverse() {
		auto tmp = player1;
		player1 = player2;
		player2 = tmp;
	}

	bool play(std::uint8_t move) {
		assert(__builtin_popcount(move) == 1 && move < (1 << 7));
		std::uint64_t offsetMove = move;
		for (auto i = 0; i < 6 && (offsetMove & player1) == 0 && (offsetMove & player2) == 0; ++i) {
			offsetMove <<= 7;
		}
		offsetMove >>= 7;
		if (offsetMove == 0) {
			return false;
		}
		player1 |= offsetMove;
		assert((player1 & player2) == 0);
		return true;
	}

	std::string print() {
		std::ostringstream tmp;
		tmp << std::bitset<42>(player1) << "\n";
		tmp << std::bitset<42>(player2) << "\n";
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

	template<std::size_t depth, class T>
	bool getScore(T storage) {
		if (playerWins(player1)) {
			return true;
		}
		Field tmp{*this};
		tmp.reverse();
		for (auto i = 0; i < 7; ++i) {
			*std::next(storage, i) = tmp;
			if constexpr (depth > 16) {
				return false;
			} else {
				if (std::next(storage, i)->play(1 << i) && std::next(storage, i)->template getScore<depth + 1>(std::next(storage, 7))) {
					return false;
				}
			}
		}
		return true;
	}

	void botMove() {
		/*Field tmp{*this};
		// Field, move (one-hot), score (moves until won/lost)
		std::vector<std::tuple<Field, std::uint_fast8_t, std::int_fast8_t>> storage{49ul - __builtin_popcountll(player1) - __builtin_popcountll(player2), {tmp, 0, 0}};
		auto pos = std::next(storage.begin());
		while (pos != storage.begin()) {
			std::get<0>(*pos).play(std::get<1>(*pos));
			if (playerWins(std::get<0>(*pos).player1)) {
				std::advance(pos, -1);
				std::get<2>(*pos) = -1;
			} else if (playerWins(std::get<0>(*pos).player2)) {
				std::advance(pos, -1);
				std::get<2>(*pos) = -1;
			}
		}*/

		Field tmp{*this};
		std::vector<Field> fields{(49ul - __builtin_popcountll(player1) - __builtin_popcountll(player2)) * 7, tmp};
		for (auto i = 0; i < 7; ++i) {
			std::cerr << "\r" << (0 + 1.0/7 * i) * 100 << "%  ";
			if (fields[i].play(1 << i) && fields[i].getScore<1>(std::next(fields.begin(), 7))) {
				play(1 << i);
				return;
			}
		}
		for (auto i = 0; i < 7; ++i) {
			if (fields[i] != tmp) {
				play(1 << i);
				return;
			}
		}
	}

	friend bool operator==(Field f1, Field f2);
	friend bool operator!=(Field f1, Field f2);
};

bool operator==(Field f1, Field f2) {
	return f1.player1 == f2.player1 && f1.player2 == f2.player2;
}

bool operator!=(Field f1, Field f2) {
	return f1.player1 != f2.player1 || f1.player2 != f2.player2;
}

bool playerWins(const std::uint64_t& player) {
	// vertical
	for (std::uint_fast8_t i = 0; i < 7; ++i) {
		if (player & (0b1ull << (21 + i)) && player & (0b1ull << (14 + i)) && __builtin_popcountll(player & (0b100000010000001000000100000010000001ull << i)) > 3) {
			if (player & (0b1ull << (7 + i))) {
				if (player & (0b1ull << i) || player & (0b1ull << (28 + i))) {
					return true;
				}
			} else if (player & (0b1ull << (28 + i)) && player & (0b1ull << (35 + i))) {
				return true;
			}
		}
	}
	// horizontal
	for (std::uint_fast8_t i = 0; i < 42; i += 7) {
		if (player & (0b1000ull << i) && __builtin_popcountll(player & (0b1111111ull << i)) > 3) {
			if (player & (0b100ull << i)) {
				if (player & (0b10ull << i)) {
					if (player & (0b1ull << i) || player & (0b10000ull << i)) {
						return true;
					}
				} else if (player & (0b10000ull << i) && player & (0b100000ull << i)) {
					return true;
				}
			} else if (player & (0b10000ull << i) && player & (0b100000ull << i) && player & (0b1000000ull << i)) {
				return true;
			}
		}
	}
	// diagonal
	for (std::uint_fast8_t i = 0; i < 42; i += 7) {
		if (player & (0b1000ull << i) && __builtin_popcountll(player & (0b1111111ull << i)) > 3) {
			if (i < 35 && player & (0b100ull << (i + 7))) {
				if (i < 28 && player & (0b10ull << (i + 14))) {
					if ((i < 21 && player & (0b1ull << (i + 21))) || (i > 0 && player & (0b10000ull << (i - 7)))) {
						return true;
					}
				} else if (i > 7 && player & (0b10000ull << (i - 7)) && player & (0b100000ull << (i - 14))) {
					return true;
				}
			} else if (i > 14 && player & (0b10000ull << (i - 7)) && player & (0b100000ull << (i - 14)) && player & (0b1000000ull << (i - 21))) {
				return true;
			}
		}
		if (player & (0b1000ull << i) && __builtin_popcountll(player & (0b1111111ull << i)) > 3) {
			if (i > 0 && player & (0b100ull << (i - 7))) {
				if (i > 7 && player & (0b10ull << (i - 14))) {
					if ((i > 14 && player & (0b1ull << (i - 21))) || (i < 35 && player & (0b10000ull << (i + 7)))) {
						return true;
					}
				} else if (i < 28 && player & (0b10000ull << (i + 7)) && player & (0b100000ull << (i + 14))) {
					return true;
				}
			} else if (i < 21 && player & (0b10000ull << (i + 7)) && player & (0b100000ull << (i + 14)) && player & (0b1000000ull << (i + 21))) {
				return true;
			}
		}
	}
	return false;
}

int main() {
	Field field;
	int input;
	while (!playerWins(field.player1) && !playerWins(field.player2)) {
		field.reverse();
		std::cout << field.print();
		std::cin >> input;
		while (!field.play(1 << (input - 1))) {
			std::cin >> input;
		}
		field.reverse();
		field.botMove();
	}
	field.reverse();
	if (playerWins(field.player1)) {
		std::cout << "Congratulations! You won!\n";
	} else if (playerWins(field.player2)) {
		std::cout << "Sorry, you lost.\n";
	}
	std::cout << field.print();
	return 0;
}