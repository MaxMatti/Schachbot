#include <array>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#pragma once

enum piece {
	None = 0,
	Empty = None, // for mind-typos. But generally Empty should be avoided because it looks like Enemy.
	Pawn = 1,
	Knight = 2,
	Bishop = 4,
	Rook = 8,
	Queen = Rook | Bishop,
	King = 16,
	Own = 32,
	Enemy = 64,
	OwnKing = Own | King,
	OwnQueen = Own | Queen,
	OwnRook = Own | Rook,
	OwnBishop = Own | Bishop,
	OwnKnight = Own | Knight,
	OwnPawn = Own | Pawn,
	EnemyKing = Enemy | King,
	EnemyQueen = Enemy | Queen,
	EnemyRook = Enemy | Rook,
	EnemyBishop = Enemy | Bishop,
	EnemyKnight = Enemy | Knight,
	EnemyPawn = Enemy | Pawn,
};

std::string getChessSymbol(piece type);
piece getChessPiece(char input);

class Board;
class Move;
class Bot;

namespace std {
	template <std::size_t size>
	struct hash<std::array<piece, size>>;
	template <std::size_t size>
	struct hash<std::array<bool, size>>;
	template <>
	struct hash<Board>;
}

class Board {
private:
	std::array<piece, 64> fields;
	std::array<bool, 4> castling;
	std::array<bool, 8> enPassant;
	std::array<bool, 2> check;
	std::array<unsigned char, 2> kingPos;
public:
	Board();
	Board(const Board & previous) : Board(previous, 0) {};
	Board(const Board & previous, const bool revert);
	Board(std::string input);
	void initEmptyField();
	std::string print() const;
	std::string display() const;

	bool checkmate() const;
	bool isCheck() const;
	bool isValid() const;
	unsigned char countPieces(const piece type) const;
	//void getNextPiece(const piece type, unsigned char *pos) const;
	unsigned char getNextPiece(const piece type, unsigned char pos) const;
	std::vector<Move> getValidMoves() const;
	std::vector<Move> getValidMoves(const unsigned char pos) const;
	bool isValidMove(const unsigned char from, const unsigned char to) const;
	bool isValidMove(const Move& move) const;
	bool isValidMove(const std::string& move) const;
	bool isThreatened(const unsigned char pos, const piece opponent = Enemy) const;
	void updateThreats(const unsigned char from, const unsigned char to);
	bool threatens(const unsigned char from, const unsigned char to) const;
	bool opensThreat(const unsigned char from, const unsigned char to) const;
	std::vector<char> getThreatRelevantPositions(const unsigned char pos) const;
	bool isCastlingAllowed(const bool which) const;
	Board applyMove(const unsigned char from, const unsigned char to, const piece turn_to, bool rotateBoard = true) const;
	Board applyMove(const Move move, bool rotateBoard = true) const;
	Board applyMove(const std::string move, bool rotateBoard = true)const;
	bool operator==(const Board& other) const;

	friend std::ostream& operator<<(std::ostream& stream, const Board& board);
	friend bool operator<(const Board& board1, const Board& board2);
	//friend bool operator==(const Board& board1, const Board& board2);
	friend Bot;
	friend std::hash<Board>;
};

std::vector<unsigned char> getBetweenFields(unsigned char from, unsigned char to);

piece operator|(const piece a, const piece b);
piece operator&(const piece a, const piece b);
piece operator^(const piece a, const piece b);
piece invertPiece(const piece input);
piece getPieceFromNotation(const char input);
std::ostream& operator<<(std::ostream& stream, const piece& obj);
std::ostream& operator<<(std::ostream& stream, const Board& board);
bool operator<(const Board& board1, const Board& board2);
//bool operator==(const Board& board1, const Board& board2);
constexpr unsigned char getDirectionSize(const unsigned char pos, const char direction);
std::unordered_map<char, unsigned char> getDirectionSizes(const unsigned char pos);

namespace std {
	template <std::size_t size>
	struct hash<std::array<piece, size>> {
		std::size_t operator()(const std::array<piece, size>& input) const {
			std::size_t result = 5;
			for (const piece& i : input) {
				result *= 7;
				result += i;
			}
			return result;
		}
	};

	template <std::size_t size>
	struct hash<std::array<bool, size>> {
		std::size_t operator()(const std::array<bool, size>& input) const {
			std::size_t result = 1;
			for (const bool& i : input) {
				result <<= 1;
				result += i;
			}
			return result;
		}
	};

	template <>
	struct hash<Board> {
		std::size_t operator()(const Board& board1) const {
			return ((std::hash<std::array<piece, 64>>()(board1.fields) * 31 + (std::hash<std::array<bool, 4>>()(board1.castling) << 1)) >> 1);
		}
	};
}
