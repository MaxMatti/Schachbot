#include <algorithm>
#include <array>
#include <cstring>
#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#define popcount __builtin_popcountll

enum Direction : std::uint64_t {
	None = 0,
	North = 1ul << 63,
	South = 1ul << 62,
	East = 1ul << 61,
	West = 1ul << 60,
};

constexpr std::uint64_t getRow(std::uint64_t pos) {
	int tz = __builtin_ctzll(pos);
	if (tz < 7) {
		return 0b1111111ul;
	} else if (tz < 14) {
		return 0b1111111ul << 7;
	} else if (tz < 21) {
		return 0b1111111ul << 14;
	} else if (tz < 28) {
		return 0b1111111ul << 21;
	} else if (tz < 35) {
		return 0b1111111ul << 28;
	} else if (tz < 42) {
		return 0b1111111ul << 35;
	} else if (tz < 49) {
		return 0b1111111ul << 42;
	} else {
		return 0ul;
	}
}

constexpr Direction oppositeDirection(Direction dir) {
	if (dir == North) {
		return South;
	} else if (dir == South) {
		return North;
	} else if (dir == East) {
		return West;
	} else if (dir == West) {
		return East;
	} else {
		return None;
	}
}

bool playerWins(std::uint64_t king) {
	return king & 1 << 24;
}

bool playerLoses(std::uint64_t king) {
	return popcount(king) == 0;
}

int countRow(std::uint64_t stones, std::uint64_t move) {
	return popcount(getRow(move) & stones);
}

constexpr std::uint64_t getCol(std::uint64_t pos) {
	int tz = __builtin_ctzll(pos);
	std::uint64_t result = 1ul | 1ul << 7 | 1ul << 14 | 1ul << 21 | 1ul << 28 | 1ul << 35 | 1ul << 42;
	return result << (tz % 7);
}

int countCol(std::uint64_t stones, std::uint64_t move) {
	return popcount(getCol(move) & stones);
}

std::string printDir(Direction dir) {
	if (dir == North) {
		return "north";
	} else if (dir == South) {
		return "south";
	} else if (dir == East) {
		return "east";
	} else if (dir == West) {
		return "west";
	} else {
		return "none";
	}
}

std::string printStone(std::uint64_t stone) {
	std::ostringstream result;
	auto coords = __builtin_ctzll(stone);
	result << static_cast<char>(coords % 7 + 'A') << coords / 7 + 1;
	return result.str();
}

std::string printStones(std::uint64_t stones, std::string delimiter = ", ") {
	std::ostringstream result;
	stones &= 0b1111111111111111111111111111111111111111111111111;
	for (; popcount(stones) > 1; stones &= ~(1ul << __builtin_ctzll(stones))) {
		result << printStone(stones) << delimiter;
	}
	if (popcount(stones) > 0) {
		result << printStone(stones);
	}
	return result.str();
}

struct Field {
	std::uint64_t king1;
	std::uint64_t king2;
	std::uint64_t stones1;
	std::uint64_t stones2;
	std::array<std::uint64_t, 6> lastMove;

	// returns wether the move is valid.
	template<bool loud>
	bool applyMove(std::uint64_t move);
	void reverse();

	template<std::size_t depth>
	void botMove();

	template<std::size_t depth>
	int botScore() const;

	std::string print() const;

private:
	template<bool loud, Direction dir>
	bool applyMoveImpl(std::uint64_t move, int distance, int maxDistance);
};

std::string Field::print() const {
	std::ostringstream tmp;
	tmp << "╔═╦═══════╦═╗\n";
	tmp << "║ ║ABCDEFG║ ║\n";
	tmp << "╠═╬═══════╬═╣\n";
	for (auto i = 0; i < 7; ++i) {
		tmp << "║" << i + 1 << "║";
		for (auto j = 0; j < 7; ++j) {
			if (stones1 & (1ull << (i * 7 + j))) {
				if (stones2 & (1ull << (i * 7 + j))) {
					tmp << "E";
				} else {
					if (king1 & (1ull << (i * 7 + j))) {
						tmp << "\033[1mX\033[0m";
					} else {
						tmp << "X";
					}
				}
			} else if (stones2 & (1ull << (i * 7 + j))) {
				if (king2 & (1ull << (i * 7 + j))) {
					tmp << "\033[1mO\033[0m";
				} else {
					tmp << "O";
				}
			} else if (i == 3 && j == 3) {
				tmp << "+";
			} else {
				tmp << " ";
			}
		}
		tmp << "║" << i + 1 << "║\n";
	}
	tmp << "╠═╬═══════╬═╣\n";
	tmp << "║ ║ABCDEFG║ ║\n";
	tmp << "╚═╩═══════╩═╝\n";
	return tmp.str();
}

template<bool loud, Direction dir>
bool Field::applyMoveImpl(std::uint64_t move, int distance, int maxDistance) {
	auto moveFunc = [](std::uint64_t pos, auto distance) {
		if constexpr (dir == North) {
			return pos >> distance * 7;
		} else if constexpr (dir == South) {
			return pos << distance * 7;
		} else if constexpr (dir == East) {
			return pos << distance * 1;
		} else if constexpr (dir == West) {
			return pos >> distance * 1;
		} else {
			static_assert(dir != dir, "This should not be instantiated.");
		}
	};

	auto distFunc = [](std::uint64_t pos1, std::uint64_t pos2) {
		if constexpr (dir == North) {
			return (__builtin_ctzll(pos1) - __builtin_ctzll(pos2)) / 7;
		} else if constexpr (dir == South) {
			return (__builtin_ctzll(pos2) - __builtin_ctzll(pos1)) / 7;
		} else if constexpr (dir == East) {
			return __builtin_ctzll(pos2) - __builtin_ctzll(pos1);
		} else if constexpr (dir == West) {
			return __builtin_ctzll(pos1) - __builtin_ctzll(pos2);
		} else {
			static_assert(dir != dir, "This should not be instantiated.");
		}
	};

	auto contFunc = [](std::uint64_t pos) {
		if constexpr (dir == North) {
			return pos != 0;
		} else if constexpr (dir == South) {
			return pos != 0 && pos < 1ul << 49;
		} else if constexpr (dir == East) {
			return pos != 0 && __builtin_ctzll(pos) % 7 != 0;
		} else if constexpr (dir == West) {
			return pos != 0 && __builtin_ctzll(pos) % 7 != 6;
		} else {
			static_assert(dir != dir, "This should not be instantiated.");
		}
	};

	if (distance > 6 || distance < 1 || distance > maxDistance) {
		if constexpr (loud) {
			std::cout << "Distance must be between 1 and 6!\n";
		}
		return false;
	}
	Field result{*this};
	for (auto& it : result.lastMove) {
		it = dir;
	}

	auto moveTo = moveFunc(move, distance);
	std::uint64_t stones1tmp = 0;
	std::uint64_t stones2tmp = 0;
	// move stones
	for (;contFunc(moveTo) && move != moveTo; move = moveFunc(move, 1)) {
		auto dist = distFunc(move, moveTo);
		if (lastMove[dist] & oppositeDirection(dir) && lastMove[dist] & move) {
			if constexpr (loud) {
				std::cout << "Cannot move stone back to its previous position!\n";
			}
			return false;
		}
		if (stones1 & move) {
			result.stones1 &= ~move;
			stones1tmp |= moveTo;
			if (king1 & move) {
				result.king1 = moveTo;
			}
			result.lastMove[dist] |= moveTo;
			moveTo = moveFunc(moveTo, 1);
		}
		if (stones2 & move) {
			result.stones2 &= ~move;
			stones2tmp |= moveTo;
			if (king2 & move) {
				result.king2 = moveTo;
			}
			result.lastMove[dist] |= moveTo;
			moveTo = moveFunc(moveTo, 1);
		}
	}
	for (;move != moveTo; move = moveFunc(move, 1)) {
		if (stones1 & move) {
			result.stones1 &= ~move;
		}
		if (stones2 & move) {
			result.stones2 &= ~move;
		}
	}
	result.stones1 |= stones1tmp;
	result.stones2 |= stones2tmp;
	result.king1 &= result.stones1;
	result.king2 &= result.stones2;
	*this = result;
	return true;
}

template<bool loud>
bool Field::applyMove(std::uint64_t move) {
	// can only move in 1 direction.
	if (popcount(move & (North | East | South | West)) != 1) {
		if constexpr (loud) {
			std::cout << "Can only move in 1 diection!\n";
		}
		return false;
	}
	// can only move 1 stone.
	if (popcount(move & 0b1111111111111111111111111111111111111111111111111) != 1) {
		if constexpr (loud) {
			std::cout << "Can only move 1 stone!\n";
		}
		return false;
	}
	// can only move own stones.
	if (popcount(move & stones1) == 0) {
		if constexpr (loud) {
			std::cout << "Can only move own stones!\nOwn stones: " << printStones(stones1) << ", Move: " << __builtin_ctzll(move) << "\n";
		}
		return false;
	}
	if (move & North) {
		move &= ~North;
		int distance = countRow(stones1 | stones2, move);
		int maxDistance = __builtin_ctzll(move) / 7;
		return applyMoveImpl<loud, North>(move, distance, maxDistance);
	} else if (move & South) {
		move &= ~South;
		int distance = countRow(stones1 | stones2, move);
		int maxDistance = 6 - __builtin_ctzll(move) / 7;
		return applyMoveImpl<loud, South>(move, distance, maxDistance);
	} else if (move & East) {
		move &= ~East;
		int distance = countCol(stones1 | stones2, move);
		int maxDistance = 6 - __builtin_ctzll(move) % 7;
		return applyMoveImpl<loud, East>(move, distance, maxDistance);
	} else if (move & West) {
		move &= ~West;
		int distance = countCol(stones1 | stones2, move);
		int maxDistance = __builtin_ctzll(move) % 7;
		return applyMoveImpl<loud, West>(move, distance, maxDistance);
	} else {
		std::cout << "Unknown error.\n";
		return false;
	}
	__builtin_unreachable();
}

template<std::size_t depth>
void Field::botMove() {
	static_assert(depth > 0, "Cannot run with depth of 0!");
	std::vector<Field> outcomes;
	outcomes.reserve(popcount(stones1) * 4);
	for (std::uint64_t stones = stones1; popcount(stones) > 1; stones &= ~(1ul << __builtin_ctzll(stones))) {
		outcomes.emplace_back(*this);
		if (outcomes.rbegin()->applyMove<false>((1ul << __builtin_ctzll(stones)) | North)) {
			outcomes.emplace_back(*this);
		}
		if (outcomes.rbegin()->applyMove<false>((1ul << __builtin_ctzll(stones)) | South)) {
			outcomes.emplace_back(*this);
		}
		if (outcomes.rbegin()->applyMove<false>((1ul << __builtin_ctzll(stones)) | East)) {
			outcomes.emplace_back(*this);
		}
		if (!outcomes.rbegin()->applyMove<false>((1ul << __builtin_ctzll(stones)) | West)) {
			outcomes.pop_back();
		}
	}
	// erase all scenarios where the enemy wins after my move.
	outcomes.erase(std::remove_if(outcomes.begin(), outcomes.end(), [](const auto& it) {
		return playerWins(it.king2) || playerLoses(it.king1);
	}), outcomes.end());
	outcomes.shrink_to_fit();
	// check for scenarios where we win the game with this move.
	for (const auto& it : outcomes) {
		if (playerWins(it.king1) || playerLoses(it.king2)) {
			*this = it;
			return;
		}
	}
	Field bestMove{*this};
	int bestScore = -100;
	for (const auto& it : outcomes) {
		int currentScore = it.template botScore<depth - 1>();
		if (currentScore > bestScore) {
			bestScore = currentScore;
			bestMove = it;
		}
	}
	*this = bestMove;
}

template<std::size_t depth>
int Field::botScore() const {
	if constexpr (depth > 0) {
		Field tmp{*this};
		tmp.botMove<depth>();
		tmp.reverse();
		int result = -tmp.botScore<depth - 1>();
		if (result < 0) {
			++result;
		} else {
			--result;
		}
		return result;
	} else {
		if (playerWins(king1) || playerLoses(king2)) {
			return 9999;
		}
		if (playerWins(king2) || playerLoses(king1)) {
			return -9999;
		}
		int result = 0;
		if (__builtin_ctzll(king1) % 7 == 3 || __builtin_ctzll(king1) / 7 == 3) {
			result += 10;
		}
		if (__builtin_ctzll(king2) % 7 == 3 || __builtin_ctzll(king2) / 7 == 3) {
			result -= 10;
		}
		result += popcount(stones1);
		result -= popcount(stones2);
		return result;
	}
}

void Field::reverse() {
	std::uint64_t tmp = stones1;
	stones1 = stones2;
	stones2 = tmp;
	tmp = king1;
	king1 = king2;
	king2 = tmp;
}

void printUsage() {
	std::cerr << "Usage:\n\t90grad [-m|--machine-readable] [-d difficulty] [-h]\n\t-m\temits output designed for automatic analysis instead of readability.\n\t-d\tsets the difficulty to the number given, should be between 1 and 23.\n\t-h\tShows this message and quits.\n";
}

using namespace std::chrono_literals;

int main(int argc [[maybe_unused]], char const *argv [[maybe_unused]] []) {
	Field field;
	field.king1 = 1ul;
	field.king2 = 1ul << 48;
	field.stones1 = 0b1111ul | 1ul << 7 | 1ul << 14 | 1ul << 21;
	field.stones2 = 0b1111ul << 45 | 1ul << 41 | 1ul << 34 | 1ul << 27;
	char inputx;
	int inputy;
	std::string inputdir;

	do {
		field.reverse();
		bool cont = true;
		Direction dir = None;
		std::cout << field.print();
		while (cont) {
			std::cin >> inputx >> inputy >> inputdir;
			cont = false;
			if (inputx < 'A' || (inputx > 'G' && inputx < 'a') || inputx > 'g') {
				std::cout << "X needs to be between A and G!\n";
				cont = true;
			} else if (inputx > 'G') {
				inputx -= 'a';
			} else {
				inputx -= 'A';
			}
			if (inputy < 1 || inputy > 7) {
				std::cout << "Y needs to be between 1 and 7!\n";
				cont = true;
			}
			std::transform(inputdir.begin(), inputdir.end(), inputdir.begin(), ::tolower);
			if (inputdir.find("up") != inputdir.npos || inputdir.find("north") != inputdir.npos) {
				dir = North;
			} else if (inputdir.find("down") != inputdir.npos || inputdir.find("south") != inputdir.npos) {
				dir = South;
			} else if (inputdir.find("right") != inputdir.npos || inputdir.find("east") != inputdir.npos) {
				dir = East;
			} else if (inputdir.find("left") != inputdir.npos || inputdir.find("west") != inputdir.npos) {
				dir = West;
			} else {
				std::cout << "No valid direction detected.\n";
				cont = true;
			}
			if (!cont) {
				if (!field.applyMove<true>(1ul << ((inputy-1) * 7 + (inputx)) | dir)) {
					std::cout << "This is not a valid move.\n";
					cont = true;
				}
			}
		}
		field.reverse();
		if (playerWins(field.king1) || playerWins(field.king2) || playerLoses(field.king1) || playerLoses(field.king2)) {
			break;
		}
		field.botMove<8>();
	} while (!playerWins(field.king1) && !playerWins(field.king2) && !playerLoses(field.king1) && !playerLoses(field.king2));
	field.reverse();
	if (playerWins(field.king1) || playerLoses(field.king2)) {
		std::cout << "Congratulations! You won!\n";
	} else if (playerWins(field.king2) || playerLoses(field.king1)) {
		std::cout << "Sorry, you lost.\n";
	} else {
		std::cout << "Seems to be a draw.\n";
	}
	std::cout << field.print();
	return 0;
}