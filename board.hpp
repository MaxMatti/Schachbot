#pragma once
#include "move.hpp"
#include "piece.hpp"

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

template <bool amIWhite>
struct Board {
    std::array<std::uint64_t, 15> figures;
    std::array<bool, 4> castling{true, true, true, true};
    bool isThreatenedByRook(const std::uint64_t pos, const piece opponent) const;
    bool isThreatenedByBishop(const std::uint64_t pos, const piece opponent) const;
    bool isThreatenedByKnight(const std::uint64_t pos, const piece opponent) const;
    bool isThreatenedByPawn(const std::uint64_t pos, const piece opponent) const;

    Board();
    template <bool hasBeenWhite>
    Board(Board<hasBeenWhite>&& previous);
    template <bool hasBeenWhite>
    Board(const Board<hasBeenWhite>& previous);
    template <bool hasBeenWhite>
    constexpr Board<amIWhite>& operator=(const Board<hasBeenWhite>& previous);

    Board(std::string input);
    void initEmptyField();

    piece at(std::uint64_t pos) const;

    bool isValid() const;
    bool isValidMove(Move move) const;
    Board<amIWhite> applyMove(Move move) const;

    std::string print() const;

    template <class F>
    constexpr void forEachKingMove(F&& func) const;
    template <class F>
    constexpr void forEachQueenMove(F&& func) const;
    template <class F>
    constexpr void forEachRookMove(F&& func) const;
    template <class F>
    constexpr void forEachBishopMove(F&& func) const;
    template <class F>
    constexpr void forEachKnightMove(F&& func) const;
    template <class F>
    constexpr void forEachPawnMove(F&& func) const;

    Move getFirstValidMove() const;

    template <class F>
    void forEachValidMove(F&& func) const;

    constexpr bool isInitialPawnPosition(std::uint64_t position) const;
};

template <bool amIWhite1, bool amIWhite2>
bool operator<(const Board<amIWhite1>& board1, const Board<amIWhite2>& board2);
// bool operator==(const Board& board1, const Board& board2);
constexpr std::uint64_t getDirectionSize(const std::uint64_t pos, const char direction);

template <bool amIWhite>
Board<amIWhite>::Board() {
    this->initEmptyField();
}

template <bool amIWhite>
template <bool hasBeenWhite>
Board<amIWhite>::Board(Board<hasBeenWhite>&& previous) {
    *this = previous;
}

template <bool amIWhite>
template <bool hasBeenWhite>
Board<amIWhite>::Board(const Board<hasBeenWhite>& previous) {
    *this = previous;
}

template <bool amIWhite>
template <bool hasBeenWhite>
constexpr Board<amIWhite>& Board<amIWhite>::operator=(const Board<hasBeenWhite>& previous) {
    if constexpr (amIWhite != hasBeenWhite) {
        figures[OwnKing] = previous.figures[EnemyKing];
        figures[OwnQueen] = previous.figures[EnemyQueen];
        figures[OwnRook] = previous.figures[EnemyRook];
        figures[OwnBishop] = previous.figures[EnemyBishop];
        figures[OwnKnight] = previous.figures[EnemyKnight];
        figures[OwnPawn] = previous.figures[EnemyPawn];
        figures[OwnFigure] = previous.figures[EnemyFigure];
        figures[EnemyKing] = previous.figures[OwnKing];
        figures[EnemyQueen] = previous.figures[OwnQueen];
        figures[EnemyRook] = previous.figures[OwnRook];
        figures[EnemyBishop] = previous.figures[OwnBishop];
        figures[EnemyKnight] = previous.figures[OwnKnight];
        figures[EnemyPawn] = previous.figures[OwnPawn];
        figures[EnemyFigure] = previous.figures[OwnFigure];
    }
    else {
        figures = previous.figures;
    }
    figures[None] = previous.figures[None];
    return *this;
}

template <bool amIWhite>
Board<amIWhite>::Board(std::string input) {
    for (auto& it : figures) {
        it = 0;
    }
    if (input.size() != 64) {
        this->initEmptyField();
        return;
    }
    for (uint i = 0; i < 64; ++i) {
        switch (getPiece(input[i])) {
        case OwnKing: figures[OwnKing] = 1ul << i; break;
        case OwnQueen: figures[OwnQueen] |= 1ul << i; break;
        case OwnRook: figures[OwnRook] |= 1ul << i; break;
        case OwnBishop: figures[OwnBishop] |= 1ul << i; break;
        case OwnKnight: figures[OwnKnight] |= 1ul << i; break;
        case OwnPawn: figures[OwnPawn] |= 1ul << i; break;
        case EnemyKing: figures[EnemyKing] = 1ul << i; break;
        case EnemyQueen: figures[EnemyQueen] |= 1ul << i; break;
        case EnemyRook: figures[EnemyRook] |= 1ul << i; break;
        case EnemyBishop: figures[EnemyBishop] |= 1ul << i; break;
        case EnemyKnight: figures[EnemyKnight] |= 1ul << i; break;
        case EnemyPawn: figures[EnemyPawn] |= 1ul << i; break;
        default: break; // TODO(mstaff): handle this error.
        }
    }
    if constexpr (amIWhite) {
        if (figures[OwnKing] != 0b00010000ul) {
            castling[0] = false;
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b00000001ul)) {
            castling[0] = false;
        }
        if (!(figures[OwnRook] & 0b10000000ul)) {
            castling[1] = false;
        }
        if (figures[EnemyKing] != 0b00010000ul << 56) {
            castling[2] = false;
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b00000001ul << 56)) {
            castling[2] = false;
        }
        if (!(figures[EnemyRook] & 0b10000000ul << 56)) {
            castling[3] = false;
        }
    }
    else {
        if (figures[OwnKing] != 0b00001000ul << 56) {
            castling[0] = false;
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b00000001ul << 56)) {
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b10000000ul << 56)) {
            castling[0] = false;
        }
        if (figures[EnemyKing] != 0b00001000ul) {
            castling[2] = false;
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b00000001ul)) {
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b10000000ul)) {
            castling[2] = false;
        }
    }
    figures[OwnFigure] = figures[OwnKing] | figures[OwnQueen] | figures[OwnRook] | figures[OwnBishop] |
        figures[OwnKnight] | figures[OwnPawn];
    figures[EnemyFigure] = figures[EnemyKing] | figures[EnemyQueen] | figures[EnemyRook] | figures[EnemyBishop] |
        figures[EnemyKnight] | figures[EnemyPawn];
    figures[None] = ~(figures[OwnFigure] | figures[EnemyFigure]);
}

template <bool amIWhite>
void Board<amIWhite>::initEmptyField() {
    if constexpr (amIWhite) {
        figures[OwnKing] = 0b00010000ul;
        figures[OwnQueen] = 0b00001000ul;
        figures[OwnRook] = 0b10000001ul;
        figures[OwnBishop] = 0b00100100ul;
        figures[OwnKnight] = 0b01000010ul;
        figures[OwnPawn] = 0b11111111ul << 8;
        figures[EnemyKing] = 0b00010000ul << 56;
        figures[EnemyQueen] = 0b00001000ul << 56;
        figures[EnemyRook] = 0b10000001ul << 56;
        figures[EnemyBishop] = 0b00100100ul << 56;
        figures[EnemyKnight] = 0b01000010ul << 56;
        figures[EnemyPawn] = 0b11111111ul << 48;
    }
    else {
        figures[OwnKing] = 0b00010000ul << 56;
        figures[OwnQueen] = 0b00001000ul << 56;
        figures[OwnRook] = 0b10000001ul << 56;
        figures[OwnBishop] = 0b00100100ul << 56;
        figures[OwnKnight] = 0b01000010ul << 56;
        figures[OwnPawn] = 0b11111111ul << 48;
        figures[EnemyKing] = 0b00010000ul;
        figures[EnemyQueen] = 0b00001000ul;
        figures[EnemyRook] = 0b10000001ul;
        figures[EnemyBishop] = 0b00100100ul;
        figures[EnemyKnight] = 0b01000010ul;
        figures[EnemyPawn] = 0b11111111ul << 8;
    }
    figures[OwnFigure] = figures[OwnKing] | figures[OwnQueen] | figures[OwnRook] | figures[OwnBishop] |
        figures[OwnKnight] | figures[OwnPawn];
    figures[EnemyFigure] = figures[EnemyKing] | figures[EnemyQueen] | figures[EnemyRook] | figures[EnemyBishop] |
        figures[EnemyKnight] | figures[EnemyPawn];
    figures[None] = ~(figures[OwnFigure] | figures[EnemyFigure]);
}

template <bool amIWhite>
piece Board<amIWhite>::at(std::uint64_t pos) const {
    auto result = std::find_if(figures.begin(), figures.end(), [&](auto fig) { return fig & pos; });
    return static_cast<piece>(result - figures.begin()); // returns AnyFigure if result == figures.end()
}

template <bool amIWhite>
bool Board<amIWhite>::isValid() const {
    return figures[OwnKing] || figures[EnemyKing];
}

template <bool amIWhite>
bool Board<amIWhite>::isValidMove(Move move) const {
    return __builtin_popcountll(move.moveFrom) == 1 && __builtin_popcountll(move.moveTo) == 1 &&
        (move.moveFrom & figures[OwnFigure]) && (move.moveTo & ~figures[OwnFigure]);
}

template <bool amIWhite>
Board<amIWhite> Board<amIWhite>::applyMove(Move move) const {
    Board<amIWhite> result{*this};
    if (__builtin_popcountll(move.moveFrom) != 1) {
        std::cout << "1";
    }
    if (__builtin_popcountll(move.moveTo) != 1) {
        std::cout << "2";
    }
    if (!(move.moveFrom & figures[OwnFigure])) {
        std::cout << "3";
    }
    if (!(move.moveTo & ~figures[OwnFigure])) {
        std::cout << "4";
    }
    if (!(__builtin_popcountll(move.moveFrom) == 1 && __builtin_popcountll(move.moveTo) == 1 &&
          (move.moveFrom & figures[OwnFigure]) && (move.moveTo & ~figures[OwnFigure]))) {
        std::cout << std::endl;
    }
    assert(isValidMove(move) && "Cannot apply invalid move!");
    result.figures[move.turnFrom] &= ~move.moveFrom;
    result.figures[move.turnTo] |= move.moveTo;
    result.figures[OwnFigure] &= ~move.moveFrom;
    result.figures[OwnFigure] |= move.moveTo;
    if (result.figures[EnemyFigure] & move.moveTo) {
        result.figures[EnemyFigure] &= ~move.moveTo;
        result.figures[EnemyKing] &= ~move.moveTo;
        result.figures[EnemyQueen] &= ~move.moveTo;
        result.figures[EnemyRook] &= ~move.moveTo;
        result.figures[EnemyBishop] &= ~move.moveTo;
        result.figures[EnemyKnight] &= ~move.moveTo;
        result.figures[EnemyPawn] &= ~move.moveTo;
    }
    result.figures[None] = ~(result.figures[OwnFigure] | result.figures[EnemyFigure]);
    return result;
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachKingMove(F&& func) const {
    auto fig = figures[OwnKing];
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        move<N, 1>(currentPos, OwnKing, func);
        move<NE, 1>(currentPos, OwnKing, func);
        move<E, 1>(currentPos, OwnKing, func);
        move<SE, 1>(currentPos, OwnKing, func);
        move<S, 1>(currentPos, OwnKing, func);
        move<SW, 1>(currentPos, OwnKing, func);
        move<W, 1>(currentPos, OwnKing, func);
        move<NW, 1>(currentPos, OwnKing, func);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachQueenMove(F&& func) const {
    auto fig = figures[OwnQueen];
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        moveUntil<N, 8>(currentPos, OwnQueen, func);
        moveUntil<E, 8>(currentPos, OwnQueen, func);
        moveUntil<S, 8>(currentPos, OwnQueen, func);
        moveUntil<W, 8>(currentPos, OwnQueen, func);
        moveUntil<NE, 8>(currentPos, OwnQueen, func);
        moveUntil<SE, 8>(currentPos, OwnQueen, func);
        moveUntil<SW, 8>(currentPos, OwnQueen, func);
        moveUntil<NW, 8>(currentPos, OwnQueen, func);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachRookMove(F&& func) const {
    auto fig = figures[OwnRook];
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        moveUntil<N, 8>(currentPos, OwnRook, func);
        moveUntil<E, 8>(currentPos, OwnRook, func);
        moveUntil<S, 8>(currentPos, OwnRook, func);
        moveUntil<W, 8>(currentPos, OwnRook, func);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachBishopMove(F&& func) const {
    auto fig = figures[OwnBishop];
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        moveUntil<NE, 8>(currentPos, OwnBishop, func);
        moveUntil<SE, 8>(currentPos, OwnBishop, func);
        moveUntil<SW, 8>(currentPos, OwnBishop, func);
        moveUntil<NW, 8>(currentPos, OwnBishop, func);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachKnightMove(F&& func) const {
    auto fig = figures[OwnKnight];
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        move<NNE, 1>(currentPos, OwnKnight, func);
        move<ENE, 1>(currentPos, OwnKnight, func);
        move<ESE, 1>(currentPos, OwnKnight, func);
        move<SSE, 1>(currentPos, OwnKnight, func);
        move<SSW, 1>(currentPos, OwnKnight, func);
        move<WSW, 1>(currentPos, OwnKnight, func);
        move<WNW, 1>(currentPos, OwnKnight, func);
        move<NNW, 1>(currentPos, OwnKnight, func);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachPawnMove(F&& func) const {
    auto moveFunc = [&](auto m) {
        if (figures[None] & m.moveTo) {
            func(m);
            return true;
        }
        return false;
    };
    auto takeFunc = [&](auto m) {
        if (figures[EnemyFigure] & m.moveTo) {
            func(m);
            return true;
        }
        return false;
    };
    auto fig = figures[OwnPawn];
    constexpr Direction moveDirection = amIWhite ? N : S;
    constexpr Direction takeDirection1 = amIWhite ? NE : SE;
    constexpr Direction takeDirection2 = amIWhite ? NW : SW;
    while (__builtin_popcountll(fig) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(fig);
        if (move<moveDirection, 1>(currentPos, OwnPawn, moveFunc) && isInitialPawnPosition(currentPos)) {
            move<moveDirection, 2>(currentPos, OwnPawn, moveFunc);
        }
        move<takeDirection1, 1>(currentPos, OwnPawn, takeFunc);
        move<takeDirection2, 1>(currentPos, OwnPawn, takeFunc);
        fig &= ~currentPos;
    }
}

template <bool amIWhite>
constexpr bool Board<amIWhite>::isInitialPawnPosition(std::uint64_t position) const {
    assert(__builtin_popcountll(position) == 1);
    if constexpr (amIWhite) {
        return (0b11111111ul << 48) & position;
    }
    else {
        return (0b11111111ul << 8) & position;
    }
}

template <bool amIWhite>
std::string Board<amIWhite>::print() const {
    std::ostringstream tmp;
    tmp << " abcdefgh \n";
    for (auto i = 0; i < 64; ++i) {
        if (i % 8 == 0) {
            tmp << 8 - i / 8;
        }
        if ((i + i / 8) % 2 == 0) {
            tmp << "\033[1;47m\033[1;30m";
        }
        else {
            tmp << "\033[1;40m\033[1;37m";
        }
        if (figures[OwnKing] & (1ul << i)) {
            tmp << "K";
        }
        else if (figures[OwnQueen] & (1ul << i)) {
            tmp << "Q";
        }
        else if (figures[OwnRook] & (1ul << i)) {
            tmp << "R";
        }
        else if (figures[OwnBishop] & (1ul << i)) {
            tmp << "B";
        }
        else if (figures[OwnKnight] & (1ul << i)) {
            tmp << "N";
        }
        else if (figures[OwnPawn] & (1ul << i)) {
            tmp << "P";
        }
        else if (figures[EnemyKing] & (1ul << i)) {
            tmp << "k";
        }
        else if (figures[EnemyQueen] & (1ul << i)) {
            tmp << "q";
        }
        else if (figures[EnemyRook] & (1ul << i)) {
            tmp << "r";
        }
        else if (figures[EnemyBishop] & (1ul << i)) {
            tmp << "b";
        }
        else if (figures[EnemyKnight] & (1ul << i)) {
            tmp << "n";
        }
        else if (figures[EnemyPawn] & (1ul << i)) {
            tmp << "p";
        }
        else {
            tmp << " ";
        }
        if (i % 8 == 7) {
            tmp << "\033[0m" << 8 - i / 8 << "\n";
        }
    }
    tmp << " abcdefgh \n";
    return tmp.str();
}

template <bool amIWhite>
std::ostream& operator<<(std::ostream& stream, const Board<amIWhite>& board) {
    stream << board.print();
    return stream;
}

// TODO(mstaff): refactor this function
template <bool amIWhite>
Move Board<amIWhite>::getFirstValidMove() const {
    Move result{};
    bool taken{false};
    auto func = [&](auto m) {
        if (!taken) {
            result = m;
            taken = true;
        }
    };
    forEachValidMove(func);
    return result;
}

template <bool amIWhite>
template <class F>
void Board<amIWhite>::forEachValidMove(F&& func) const {
    auto conditionalFunc = [&](auto m) {
        if (figures[None] & m.moveTo) {
            func(m);
            return true;
        }
        else if (figures[EnemyFigure] & m.moveTo) {
            func(m);
            return false;
        }
        else if (figures[OwnFigure] & m.moveTo) {
            return false;
        }
        else {
            assert(false && "Memory error.");
        }
    };
    forEachPawnMove(conditionalFunc);
    forEachKnightMove(conditionalFunc);
    forEachBishopMove(conditionalFunc);
    forEachRookMove(conditionalFunc);
    forEachQueenMove(conditionalFunc);
    forEachKingMove(conditionalFunc);
}
