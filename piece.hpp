#pragma once

#include <array>
#include <cstdint>

enum piece : std::size_t {
    None = 0,
    WhiteKing = 1,
    WhiteQueen = 2,
    WhiteRook = 3,
    WhiteBishop = 4,
    WhiteKnight = 5,
    WhitePawn = 6,
    WhiteFigure = 7,
    BlackKing = 8,
    BlackQueen = 9,
    BlackRook = 10,
    BlackBishop = 11,
    BlackKnight = 12,
    BlackPawn = 13,
    BlackFigure = 14,
    AnyFigure = 15,
};

constexpr const char* getChessSymbol(piece type) {
    switch (type) {
    case None: return " ";
    case BlackKing: return "\033[1;33m\u265A\033[0m";
    case BlackQueen: return "\033[1;33m\u265B\033[0m";
    case BlackRook: return "\033[1;33m\u265C\033[0m";
    case BlackBishop: return "\033[1;33m\u265D\033[0m";
    case BlackKnight: return "\033[1;33m\u265E\033[0m";
    case BlackPawn: return "\033[1;33m\u265F\033[0m";
    case WhiteKing: return "\033[1;37m\u265A\033[0m";
    case WhiteQueen: return "\033[1;37m\u265B\033[0m";
    case WhiteRook: return "\033[1;37m\u265C\033[0m";
    case WhiteBishop: return "\033[1;37m\u265D\033[0m";
    case WhiteKnight: return "\033[1;37m\u265E\033[0m";
    case WhitePawn: return "\033[1;37m\u265F\033[0m";
    default: return "x";
    }
}

constexpr char getChessChar(piece type) {
    switch (type) {
    case None: return ' ';
    case BlackKing: return 'k';
    case BlackQueen: return 'q';
    case BlackRook: return 'r';
    case BlackBishop: return 'b';
    case BlackKnight: return 'n';
    case BlackPawn: return 'p';
    case BlackFigure: return 'f';
    case WhiteKing: return 'K';
    case WhiteQueen: return 'Q';
    case WhiteRook: return 'R';
    case WhiteBishop: return 'B';
    case WhiteKnight: return 'N';
    case WhitePawn: return 'P';
    case WhiteFigure: return 'F';
    default: return 'x';
    }
}

constexpr piece getPiece(char input) {
    switch (input) {
    case ' ': return None;
    case '-': return None;

    case 'k': return BlackKing;

    case 'q':
    case 'd': return BlackQueen;

    case 'r':
    case 't': return BlackRook;

    case 'b':
    case 'l': return BlackBishop;

    case 'n':
    case 's': return BlackKnight;

    case 'p': return BlackPawn;

    case 'K': return WhiteKing;

    case 'Q':
    case 'D': return WhiteQueen;

    case 'R':
    case 'T': return WhiteRook;

    case 'B':
    case 'L': return WhiteBishop;

    case 'N':
    case 'S': return WhiteKnight;

    case 'P': return WhitePawn;

    default: return None;
    }
}

constexpr bool isKing(piece input) { return input == WhiteKing || input == BlackKing; }

constexpr bool isQueen(piece input) { return input == WhiteQueen || input == BlackQueen; }

constexpr bool isRook(piece input) { return input == WhiteRook || input == BlackRook; }

constexpr bool isBishop(piece input) { return input == WhiteBishop || input == BlackBishop; }

constexpr bool isKnight(piece input) { return input == WhiteKnight || input == BlackKnight; }

constexpr bool isPawn(piece input) { return input == WhitePawn || input == BlackPawn; }

constexpr bool isWhite(piece input) {
    return input == WhiteKing || input == WhiteQueen || input == WhiteRook || input == WhiteBishop ||
        input == WhiteKnight || input == WhitePawn || input == WhiteFigure;
}

constexpr bool isBlack(piece input) {
    return input == BlackKing || input == BlackQueen || input == BlackRook || input == BlackBishop ||
        input == BlackKnight || input == BlackPawn || input == BlackFigure;
}

constexpr piece invertPiece(const piece input) {
    switch (input) {
    case None: return AnyFigure;
    case WhiteKing: return BlackKing;
    case WhiteQueen: return BlackQueen;
    case WhiteRook: return BlackRook;
    case WhiteBishop: return BlackBishop;
    case WhiteKnight: return BlackKnight;
    case WhitePawn: return BlackPawn;
    case WhiteFigure: return BlackFigure;
    case BlackKing: return WhiteKing;
    case BlackQueen: return WhiteQueen;
    case BlackRook: return WhiteRook;
    case BlackBishop: return WhiteBishop;
    case BlackKnight: return WhiteKnight;
    case BlackPawn: return WhitePawn;
    case BlackFigure: return WhiteFigure;
    case AnyFigure: return None;
    default: return None;
    }
}

namespace std {
template <std::size_t size>
struct hash<std::array<piece, size>>;
}

std::ostream& operator<<(std::ostream& stream, const piece& obj);

std::string printPos(std::uint64_t pos);

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
} // namespace std
