#pragma once

#include <array>
#include <cstdint>

enum piece : std::size_t {
    None = 0,
    OwnKing = 1,
    OwnQueen = 2,
    OwnRook = 3,
    OwnBishop = 4,
    OwnKnight = 5,
    OwnPawn = 6,
    OwnFigure = 7,
    EnemyKing = 8,
    EnemyQueen = 9,
    EnemyRook = 10,
    EnemyBishop = 11,
    EnemyKnight = 12,
    EnemyPawn = 13,
    EnemyFigure = 14,
    AnyFigure = 15,
};

constexpr const char* getChessSymbol(piece type) {
    switch (type) {
    case None: return " ";
    case EnemyKing: return "\033[1;33m\u265A\033[0m";
    case EnemyQueen: return "\033[1;33m\u265B\033[0m";
    case EnemyRook: return "\033[1;33m\u265C\033[0m";
    case EnemyBishop: return "\033[1;33m\u265D\033[0m";
    case EnemyKnight: return "\033[1;33m\u265E\033[0m";
    case EnemyPawn: return "\033[1;33m\u265F\033[0m";
    case OwnKing: return "\033[1;37m\u265A\033[0m";
    case OwnQueen: return "\033[1;37m\u265B\033[0m";
    case OwnRook: return "\033[1;37m\u265C\033[0m";
    case OwnBishop: return "\033[1;37m\u265D\033[0m";
    case OwnKnight: return "\033[1;37m\u265E\033[0m";
    case OwnPawn: return "\033[1;37m\u265F\033[0m";
    default: return "x";
    }
}

constexpr char getChessChar(piece type) {
    switch (type) {
    case None: return ' ';
    case EnemyKing: return 'k';
    case EnemyQueen: return 'q';
    case EnemyRook: return 'r';
    case EnemyBishop: return 'b';
    case EnemyKnight: return 'n';
    case EnemyPawn: return 'p';
    case OwnKing: return 'K';
    case OwnQueen: return 'Q';
    case OwnRook: return 'R';
    case OwnBishop: return 'B';
    case OwnKnight: return 'N';
    case OwnPawn: return 'P';
    default: return 'x';
    }
}

constexpr piece getPiece(char input) {
    switch (input) {
    case ' ': return None;

    case 'k': return EnemyKing;

    case 'q':
    case 'd': return EnemyQueen;

    case 'r':
    case 't': return EnemyRook;

    case 'b':
    case 'l': return EnemyBishop;

    case 'n':
    case 's': return EnemyKnight;

    case 'p': return EnemyPawn;

    case 'K': return OwnKing;

    case 'Q':
    case 'D': return OwnQueen;

    case 'R':
    case 'T': return OwnRook;

    case 'B':
    case 'L': return OwnBishop;

    case 'N':
    case 'S': return OwnKnight;

    case 'P': return OwnPawn;

    default: return None;
    }
}

constexpr bool isKing(piece input) { return input == OwnKing || input == EnemyKing; }

constexpr bool isQueen(piece input) { return input == OwnQueen || input == EnemyQueen; }

constexpr bool isRook(piece input) { return input == OwnRook || input == EnemyRook; }

constexpr bool isBishop(piece input) { return input == OwnBishop || input == EnemyBishop; }

constexpr bool isKnight(piece input) { return input == OwnKnight || input == EnemyKnight; }

constexpr bool isPawn(piece input) { return input == OwnPawn || input == EnemyPawn; }

constexpr bool isOwn(piece input) {
    return input == OwnKing || input == OwnQueen || input == OwnRook || input == OwnBishop || input == OwnKnight ||
        input == OwnPawn || input == OwnFigure;
}

constexpr bool isEnemy(piece input) {
    return input == EnemyKing || input == EnemyQueen || input == EnemyRook || input == EnemyBishop ||
        input == EnemyKnight || input == EnemyPawn || input == EnemyFigure;
}

constexpr piece invertPiece(const piece input) {
    switch (input) {
    case None: return AnyFigure;
    case OwnKing: return EnemyKing;
    case OwnQueen: return EnemyQueen;
    case OwnRook: return EnemyRook;
    case OwnBishop: return EnemyBishop;
    case OwnKnight: return EnemyKnight;
    case OwnPawn: return EnemyPawn;
    case OwnFigure: return EnemyFigure;
    case EnemyKing: return OwnKing;
    case EnemyQueen: return OwnQueen;
    case EnemyRook: return OwnRook;
    case EnemyBishop: return OwnBishop;
    case EnemyKnight: return OwnKnight;
    case EnemyPawn: return OwnPawn;
    case EnemyFigure: return OwnFigure;
    case AnyFigure: return None;
    default: return None;
    }
}

namespace std {
template <std::size_t size>
struct hash<std::array<piece, size>>;
}

std::ostream& operator<<(std::ostream& stream, const piece& obj);

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