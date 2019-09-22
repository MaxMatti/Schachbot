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

constexpr const static std::uint64_t whiteKingStartPos = 0b00010000ul;
constexpr const static std::uint64_t blackKingStartPos = 0b00010000ul << 56;
constexpr const static std::uint64_t castling1Fields = 0b01100000ul;
constexpr const static std::uint64_t castling2Fields = 0b00001110ul;
constexpr const static std::uint64_t castling3Fields = 0b01100000ul << 56;
constexpr const static std::uint64_t castling4Fields = 0b00001110ul << 56;

template <bool amIWhite>
struct Board {
    std::array<std::uint64_t, 16> figures;
    std::array<bool, 4> castling{true, true, true, true};

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

    piece figureAt(std::uint64_t pos) const;

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

    template <piece fig, class F>
    constexpr bool forEachThreatByFigure(F&& func, std::uint64_t positions) const;

    template <class F>
    constexpr void forEachThreat(std::uint64_t positions, F&& func) const;

    constexpr bool isThreatened(std::uint64_t positions) const;
};

template <bool amIWhite1, bool amIWhite2>
bool operator<(const Board<amIWhite1>& board1, const Board<amIWhite2>& board2);
template <bool amIWhite>
bool operator==(const Board<amIWhite>& board1, const Board<amIWhite>& board2) {
    return board1.figures == board2.figures && board1.castling == board2.castling;
}
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
    figures[AnyFigure] = previous.figures[AnyFigure];
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
    figures[AnyFigure] = figures[OwnFigure] | figures[EnemyFigure];
    figures[None] = ~figures[AnyFigure];
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
    figures[AnyFigure] = figures[OwnFigure] | figures[EnemyFigure];
    figures[None] = ~figures[AnyFigure];
}

template <bool amIWhite>
piece Board<amIWhite>::at(std::uint64_t pos) const {
    auto result = std::find_if(figures.begin(), figures.end(), [&](auto fig) { return fig & pos; });
    return static_cast<piece>(result - figures.begin()); // returns AnyFigure if result == figures.end()
}

template <bool amIWhite>
bool Board<amIWhite>::isValid() const {
    return figures[OwnKing] && figures[EnemyKing];
}

template <bool amIWhite>
bool Board<amIWhite>::isValidMove(Move move) const {
    return __builtin_popcountll(move.moveFrom) == 1 && __builtin_popcountll(move.moveTo) == 1 &&
        (move.moveFrom & figures[OwnFigure]) && (move.moveTo & ~figures[OwnFigure]);
}

template <bool amIWhite>
Board<amIWhite> Board<amIWhite>::applyMove(Move move) const {
    Board<amIWhite> result{*this};
    if (!isValidMove(move)) {
        std::cout << "Trying to apply invalid move:\n" << *this << move << std::endl;
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
    if (isKing(move.turnFrom)) {
        if (move.moveFrom == whiteKingStartPos) {
            if (move.moveTo == (whiteKingStartPos << 2)) {
                assert(castling[0] && (figures[None] & castling1Fields) == castling1Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~0b10000000ul;
                result.figures[OwnRook] |= 0b00100000ul;
                result.figures[OwnFigure] &= ~0b10000000ul;
                result.figures[OwnFigure] |= 0b00100000ul;
            }
            else if (move.moveTo == whiteKingStartPos >> 3) {
                assert(castling[1] && (figures[None] & castling2Fields) == castling2Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~0b00000001ul;
                result.figures[OwnRook] |= 0b00000100ul;
                result.figures[OwnFigure] &= ~0b00000001ul;
                result.figures[OwnFigure] |= 0b00000100ul;
            }
        }
        if (move.moveFrom == blackKingStartPos) {
            if (move.moveTo == (blackKingStartPos << 2)) {
                assert(castling[0] && (figures[None] & castling3Fields) == castling3Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~0b10000000ul;
                result.figures[OwnRook] |= 0b00100000ul;
                result.figures[OwnFigure] &= ~0b10000000ul;
                result.figures[OwnFigure] |= 0b00100000ul;
            }
            else if (move.moveTo == blackKingStartPos >> 3) {
                assert(castling[1] && (figures[None] & castling4Fields) == castling4Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~0b00000001ul;
                result.figures[OwnRook] |= 0b00000100ul;
                result.figures[OwnFigure] &= ~0b00000001ul;
                result.figures[OwnFigure] |= 0b00000100ul;
            }
        }
        result.castling[2] = false;
        result.castling[3] = false;
    }
    else {
        result.castling[2] = castling[0];
        result.castling[3] = castling[1];
    }
    result.castling[0] = castling[2];
    result.castling[1] = castling[3];
    result.figures[AnyFigure] = result.figures[OwnFigure] | result.figures[EnemyFigure];
    result.figures[None] = ~result.figures[AnyFigure];
    return result;
}

template <bool amIWhite>
piece Board<amIWhite>::figureAt(std::uint64_t pos) const {
    assert(__builtin_popcountll(pos) == 1);
    for (size_t i = 0; i < figures.size(); ++i) {
        if (figures[i] & pos) {
            return static_cast<piece>(i);
        }
    }
    return None;
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachKingMove(F&& func) const {
    forEachPos(figures[OwnKing], [&](std::uint64_t currentPos) {
        checkedMove<N, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<NE, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<E, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<SE, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<S, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<SW, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<W, 1>(currentPos, OwnKing, OwnKing, func);
        checkedMove<NW, 1>(currentPos, OwnKing, OwnKing, func);
        if (castling[0]) {
            if (currentPos == whiteKingStartPos && (figures[OwnRook] & 0b10000000ul) &&
                (figures[None] & castling1Fields) == castling1Fields && !isThreatened(whiteKingStartPos) &&
                !isThreatened(whiteKingStartPos << 1) && !isThreatened(whiteKingStartPos << 2)) {
                checkedMove<EE, 1>(currentPos, OwnKing, OwnKing, func);
            }
            else if (
                currentPos == blackKingStartPos && (figures[OwnRook] & 0b10000000ul << 56) &&
                (figures[None] & castling3Fields) == castling3Fields && !isThreatened(blackKingStartPos) &&
                !isThreatened(blackKingStartPos << 1) && !isThreatened(blackKingStartPos << 2)) {
                checkedMove<EE, 1>(currentPos, OwnKing, OwnKing, func);
            }
        }
        if (castling[1]) {
            if (currentPos == whiteKingStartPos && (figures[OwnRook] & 0b00000001ul) &&
                (figures[None] & castling2Fields) == castling2Fields && !isThreatened(whiteKingStartPos) &&
                !isThreatened(whiteKingStartPos >> 1) && !isThreatened(whiteKingStartPos >> 2) &&
                !isThreatened(whiteKingStartPos >> 3)) {
                checkedMove<WWW, 1>(currentPos, OwnKing, OwnKing, func);
            }
            else if (
                currentPos == blackKingStartPos && (figures[OwnRook] & 0b00000001ul << 56) &&
                (figures[None] & castling4Fields) == castling4Fields && !isThreatened(blackKingStartPos) &&
                !isThreatened(blackKingStartPos >> 1) && !isThreatened(blackKingStartPos >> 2) &&
                !isThreatened(blackKingStartPos >> 3)) {
                checkedMove<WWW, 1>(currentPos, OwnKing, OwnKing, func);
            }
        }
        return true;
    });
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachQueenMove(F&& func) const {
    forEachPos(figures[OwnQueen], [&](std::uint64_t currentPos) {
        checkedMoveUntil<N, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<E, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<S, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<W, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<NE, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<SE, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<SW, 8>(currentPos, OwnQueen, OwnQueen, func);
        checkedMoveUntil<NW, 8>(currentPos, OwnQueen, OwnQueen, func);
        return true;
    });
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachRookMove(F&& func) const {
    forEachPos(figures[OwnRook], [&](std::uint64_t currentPos) {
        checkedMoveUntil<N, 8>(currentPos, OwnRook, OwnRook, func);
        checkedMoveUntil<E, 8>(currentPos, OwnRook, OwnRook, func);
        checkedMoveUntil<S, 8>(currentPos, OwnRook, OwnRook, func);
        checkedMoveUntil<W, 8>(currentPos, OwnRook, OwnRook, func);
        return true;
    });
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachBishopMove(F&& func) const {
    forEachPos(figures[OwnBishop], [&](std::uint64_t currentPos) {
        checkedMoveUntil<NE, 8>(currentPos, OwnBishop, OwnBishop, func);
        checkedMoveUntil<SE, 8>(currentPos, OwnBishop, OwnBishop, func);
        checkedMoveUntil<SW, 8>(currentPos, OwnBishop, OwnBishop, func);
        checkedMoveUntil<NW, 8>(currentPos, OwnBishop, OwnBishop, func);
        return true;
    });
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachKnightMove(F&& func) const {
    forEachPos(figures[OwnKnight], [&](std::uint64_t currentPos) {
        checkedMove<NNE, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<ENE, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<ESE, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<SSE, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<SSW, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<WSW, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<WNW, 1>(currentPos, OwnKnight, OwnKnight, func);
        checkedMove<NNW, 1>(currentPos, OwnKnight, OwnKnight, func);
        return true;
    });
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
    constexpr Direction moveDirection = amIWhite ? N : S;
    constexpr Direction takeDirection1 = amIWhite ? NE : SE;
    constexpr Direction takeDirection2 = amIWhite ? NW : SW;
    forEachPos(figures[OwnPawn], [&](std::uint64_t currentPos) {
        if (isInitialPawnPosition<amIWhite>(currentPos)) {
            if (checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnPawn, moveFunc)) {
                checkedMove<moveDirection, 2>(currentPos, OwnPawn, OwnPawn, moveFunc);
            }
        }
        else if (isInitialPawnPosition<!amIWhite>(currentPos)) {
            checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnQueen, moveFunc);
            checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnRook, moveFunc);
            checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnBishop, moveFunc);
            checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnKnight, moveFunc);
        }
        else {
            checkedMove<moveDirection, 1>(currentPos, OwnPawn, OwnPawn, moveFunc);
        }
        checkedMove<takeDirection1, 1>(currentPos, OwnPawn, OwnPawn, takeFunc);
        checkedMove<takeDirection2, 1>(currentPos, OwnPawn, OwnPawn, takeFunc);
        return true;
    });
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
        return false;
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
            return false;
        }
    };
    auto conditionalKingFunc = [&](auto m) {
        if (figures[None] & m.moveTo) {
            if (!isThreatened(m.moveTo)) {
                func(m);
            }
            return true;
        }
        else if (figures[EnemyFigure] & m.moveTo) {
            if (!isThreatened(m.moveTo)) {
                func(m);
            }
            return false;
        }
        else if (figures[OwnFigure] & m.moveTo) {
            return false;
        }
        else {
            assert(false && "Memory error.");
            return false;
        }
    };
    forEachPawnMove(conditionalFunc);
    forEachKnightMove(conditionalFunc);
    forEachBishopMove(conditionalFunc);
    forEachRookMove(conditionalFunc);
    forEachQueenMove(conditionalFunc);
    forEachKingMove(conditionalKingFunc);
}

template <bool amIWhite>
template <piece fig, class F>
constexpr bool Board<amIWhite>::forEachThreatByFigure(F&& func, std::uint64_t positions) const {
    return forEachPos(figures[fig], [&](std::uint64_t from) {
        return forEachPos(positions, [&](std::uint64_t to) {
            // TODO(mstaff): function naming
            if (::isValidMove<amIWhite, fig>(from, to, figures[AnyFigure])) {
                return func(from);
            }
            else {
                return true;
            }
        });
    });
}

template <bool amIWhite>
template <class F>
constexpr void Board<amIWhite>::forEachThreat(std::uint64_t positions, F&& func) const {
    forEachThreatByFigure<EnemyKing>(std::forward<F>(func), positions) &&
        forEachThreatByFigure<EnemyQueen>(std::forward<F>(func), positions) &&
        forEachThreatByFigure<EnemyRook>(std::forward<F>(func), positions) &&
        forEachThreatByFigure<EnemyBishop>(std::forward<F>(func), positions) &&
        forEachThreatByFigure<EnemyKnight>(std::forward<F>(func), positions) &&
        forEachThreatByFigure<EnemyPawn>(std::forward<F>(func), positions);
}

template <bool amIWhite>
constexpr bool Board<amIWhite>::isThreatened(std::uint64_t positions) const {
    bool result = false;
    forEachThreat(positions, [&](auto) {
        result = true;
        return false;
    });
    return result;
}
