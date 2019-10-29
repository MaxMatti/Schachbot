#pragma once
#include "move.hpp"
#include "piece.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

template <bool amIWhite>
struct Board {
    std::array<std::uint64_t, 16> figures;
    std::array<bool, 4> castling{true, true, true, true};

    constexpr const static piece OwnKing = amIWhite ? WhiteKing : BlackKing;
    constexpr const static piece OwnQueen = amIWhite ? WhiteQueen : BlackQueen;
    constexpr const static piece OwnRook = amIWhite ? WhiteRook : BlackRook;
    constexpr const static piece OwnBishop = amIWhite ? WhiteBishop : BlackBishop;
    constexpr const static piece OwnKnight = amIWhite ? WhiteKnight : BlackKnight;
    constexpr const static piece OwnPawn = amIWhite ? WhitePawn : BlackPawn;
    constexpr const static piece OwnFigure = amIWhite ? WhiteFigure : BlackFigure;

    constexpr const static piece EnemyKing = amIWhite ? BlackKing : WhiteKing;
    constexpr const static piece EnemyQueen = amIWhite ? BlackQueen : WhiteQueen;
    constexpr const static piece EnemyRook = amIWhite ? BlackRook : WhiteRook;
    constexpr const static piece EnemyBishop = amIWhite ? BlackBishop : WhiteBishop;
    constexpr const static piece EnemyKnight = amIWhite ? BlackKnight : WhiteKnight;
    constexpr const static piece EnemyPawn = amIWhite ? BlackPawn : WhitePawn;
    constexpr const static piece EnemyFigure = amIWhite ? BlackFigure : WhiteFigure;

    constexpr static bool isOwn(piece input) {
        if constexpr (amIWhite) {
            return isWhite(input);
        }
        else {
            return isBlack(input);
        }
    }

    constexpr static bool isEnemy(piece input) {
        if constexpr (amIWhite) {
            return isBlack(input);
        }
        else {
            return isWhite(input);
        }
    }

    Board();
    template <bool hasBeenWhite>
    Board(Board<hasBeenWhite>&& previous);
    template <bool hasBeenWhite>
    Board(const Board<hasBeenWhite>& previous);
    template <bool hasBeenWhite>
    constexpr Board<amIWhite>& operator=(Board<hasBeenWhite>&& previous);
    template <bool hasBeenWhite>
    constexpr Board<amIWhite>& operator=(const Board<hasBeenWhite>& previous);

    Board(std::string input);
    void initEmptyField();

    piece at(std::uint64_t pos) const;

    void fillCaches();
    bool isCacheCoherent() const;

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
Board<amIWhite>::Board(Board<hasBeenWhite>&& previous)
    : figures(previous.figures)
    , castling(previous.castling) {}

template <bool amIWhite>
template <bool hasBeenWhite>
Board<amIWhite>::Board(const Board<hasBeenWhite>& previous)
    : figures(previous.figures)
    , castling(previous.castling) {}

template <bool amIWhite>
template <bool hasBeenWhite>
constexpr Board<amIWhite>& Board<amIWhite>::operator=(Board<hasBeenWhite>&& previous) {
    figures = previous.figures;
    castling = previous.castling;
    return *this;
}

template <bool amIWhite>
template <bool hasBeenWhite>
constexpr Board<amIWhite>& Board<amIWhite>::operator=(const Board<hasBeenWhite>& previous) {
    figures = previous.figures;
    castling = previous.castling;
    return *this;
}

template <bool amIWhite>
Board<amIWhite>::Board(std::string input) {
    for (auto& it : figures) {
        it = 0;
    }
    for (std::size_t i = 0; i < input.size(); ++i) {
        // TODO(mstaff): better way to check wether input[i] is a digit
        if (input[i] >= '0' && input[i] <= '9') {
            input.replace(i, 1, std::string(static_cast<size_t>(input[i] - '0'), ' '));
        }
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
    castling[0] = true;
    castling[1] = true;
    castling[2] = true;
    castling[3] = true;
    if constexpr (amIWhite) {
        if (figures[OwnKing] != whiteKingStartPos) {
            castling[0] = false;
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b00000001ul << 56)) {
            castling[0] = false;
        }
        if (!(figures[OwnRook] & 0b10000000ul << 56)) {
            castling[1] = false;
        }
        if (figures[EnemyKing] != blackKingStartPos) {
            castling[2] = false;
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b00000001ul)) {
            castling[2] = false;
        }
        if (!(figures[EnemyRook] & 0b10000000ul)) {
            castling[3] = false;
        }
    }
    else {
        if (figures[OwnKing] != 0b00001000ul) {
            castling[0] = false;
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b00000001ul)) {
            castling[1] = false;
        }
        if (!(figures[OwnRook] & 0b10000000ul)) {
            castling[0] = false;
        }
        if (figures[EnemyKing] != 0b00001000ul << 56) {
            castling[2] = false;
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b00000001ul << 56)) {
            castling[3] = false;
        }
        if (!(figures[EnemyRook] & 0b10000000ul << 56)) {
            castling[2] = false;
        }
    }
    fillCaches();
}

template <bool amIWhite>
void Board<amIWhite>::initEmptyField() {
    if constexpr (amIWhite) {
        figures[OwnKing] = whiteKingStartPos;
        figures[OwnQueen] = 0b00001000ul;
        figures[OwnRook] = 0b10000001ul;
        figures[OwnBishop] = 0b00100100ul;
        figures[OwnKnight] = 0b01000010ul;
        figures[OwnPawn] = 0b11111111ul << 8;
        figures[EnemyKing] = blackKingStartPos;
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
    fillCaches();
}

template <bool amIWhite>
piece Board<amIWhite>::at(std::uint64_t pos) const {
    auto result = std::find_if(figures.begin(), figures.end(), [&](auto fig) { return fig & pos; });
    return static_cast<piece>(result - figures.begin()); // returns AnyFigure if result == figures.end()
}

template <bool amIWhite>
void Board<amIWhite>::fillCaches() {
    figures[OwnFigure] = figures[OwnKing] | figures[OwnQueen] | figures[OwnRook] | figures[OwnBishop] |
        figures[OwnKnight] | figures[OwnPawn];
    figures[EnemyFigure] = figures[EnemyKing] | figures[EnemyQueen] | figures[EnemyRook] | figures[EnemyBishop] |
        figures[EnemyKnight] | figures[EnemyPawn];
    figures[AnyFigure] = figures[OwnFigure] | figures[EnemyFigure];
    figures[None] = ~figures[AnyFigure];
}

template <bool amIWhite>
bool Board<amIWhite>::isCacheCoherent() const {
    return figures[OwnFigure] ==
        (figures[OwnKing] | figures[OwnQueen] | figures[OwnRook] | figures[OwnBishop] | figures[OwnKnight] |
         figures[OwnPawn]) &&
        figures[EnemyFigure] ==
        (figures[EnemyKing] | figures[EnemyQueen] | figures[EnemyRook] | figures[EnemyBishop] | figures[EnemyKnight] |
         figures[EnemyPawn]) &&
        (figures[OwnFigure] & figures[EnemyFigure]) == 0ul &&
        figures[AnyFigure] == (figures[OwnFigure] | figures[EnemyFigure]) && figures[None] == ~figures[AnyFigure];
}

template <bool amIWhite>
bool Board<amIWhite>::isValid() const {
    return figures[OwnKing] && figures[EnemyKing];
}

template <bool amIWhite>
bool Board<amIWhite>::isValidMove(Move move) const {
    return __builtin_popcountll(move.moveFrom) == 1 && __builtin_popcountll(move.moveTo) == 1 &&
        (move.moveFrom & figures[OwnFigure]) && (move.moveTo & ~figures[OwnFigure]) && isOwn(move.turnFrom) &&
        isOwn(move.turnTo);
}

template <bool amIWhite>
void extensiveTests(const Board<amIWhite>& result) {
    if (result.figures[result.OwnFigure] !=
        (result.figures[result.OwnKing] | result.figures[result.OwnQueen] | result.figures[result.OwnRook] |
         result.figures[result.OwnBishop] | result.figures[result.OwnKnight] | result.figures[result.OwnPawn])) {
        std::cout << "1\n" << result << std::endl;
    }
    if (result.figures[result.EnemyFigure] !=
        (result.figures[result.EnemyKing] | result.figures[result.EnemyQueen] | result.figures[result.EnemyRook] |
         result.figures[result.EnemyBishop] | result.figures[result.EnemyKnight] | result.figures[result.EnemyPawn])) {
        std::cout << "2\n" << result << std::endl;
    }
    if ((result.figures[result.OwnFigure] & result.figures[result.EnemyFigure]) != 0ul) {
        std::cout << "3\n" << result << std::endl;
    }
    if (result.figures[AnyFigure] != (result.figures[result.OwnFigure] | result.figures[result.EnemyFigure])) {
        std::cout << "4\n" << result << std::endl;
    }
    if (result.figures[None] != ~result.figures[AnyFigure]) {
        std::cout << "5\n" << result << std::endl;
    }
    for (auto it : result.figures) {
        for (std::uint64_t i = 1ul << 63; i != 0; i >>= 1) {
            std::cout << ((it & i) ? "1" : "0");
        }
        std::cout << " " << it << std::endl;
    }
    auto it = result.figures[result.OwnFigure];
    for (std::uint64_t i = 1ul << 63; i != 0; i >>= 1) {
        std::cout << ((it & i) ? "1" : "0");
    }
    std::cout << " " << it << std::endl;
    it = result.figures[result.EnemyFigure];
    for (std::uint64_t i = 1ul << 63; i != 0; i >>= 1) {
        std::cout << ((it & i) ? "1" : "0");
    }
    std::cout << " " << it << std::endl;
    it = (result.figures[result.OwnFigure] & result.figures[result.EnemyFigure]);
    for (std::uint64_t i = 1ul << 63; i != 0; i >>= 1) {
        std::cout << ((it & i) ? "1" : "0");
    }
    std::cout << " " << it << std::endl << std::endl;
}

template <bool amIWhite>
Board<amIWhite> Board<amIWhite>::applyMove(Move move) const {
    assert(isCacheCoherent());
    Board<amIWhite> result{*this};
    auto tmp = isValidMove(move) && ::isValidMove(move, figures[AnyFigure]);
    if (!tmp) {
        std::cout << "Trying to apply invalid move:\n" << *this << move << std::endl;
    }
    assert(tmp && "Cannot apply invalid move!");
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
        if (move.moveFrom == blackKingStartPos) {
            if (move.moveTo == (blackKingStartPos << 2)) {
                assert(castling[0] && (figures[None] & castling1Fields) == castling1Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~(0b10000000ul << 56);
                result.figures[OwnRook] |= 0b00100000ul << 56;
                result.figures[OwnFigure] &= ~(0b10000000ul << 56);
                result.figures[OwnFigure] |= 0b00100000ul << 56;
            }
            else if (move.moveTo == blackKingStartPos >> 3) {
                assert(castling[1] && (figures[None] & castling2Fields) == castling2Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~(0b00000001ul << 56);
                result.figures[OwnRook] |= 0b00000100ul << 56;
                result.figures[OwnFigure] &= ~(0b00000001ul << 56);
                result.figures[OwnFigure] |= 0b00000100ul << 56;
            }
        }
        else if (move.moveFrom == whiteKingStartPos) {
            if (move.moveTo == (whiteKingStartPos << 2)) {
                assert(castling[0] && (figures[None] & castling3Fields) == castling3Fields && "Castling is blocked!");
                result.figures[OwnRook] &= ~0b10000000ul;
                result.figures[OwnRook] |= 0b00100000ul;
                result.figures[OwnFigure] &= ~0b10000000ul;
                result.figures[OwnFigure] |= 0b00100000ul;
            }
            else if (move.moveTo == whiteKingStartPos >> 3) {
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
    assert(isCacheCoherent());
    result.figures[AnyFigure] = result.figures[OwnFigure] | result.figures[EnemyFigure];
    result.figures[None] = ~result.figures[AnyFigure];
    if (!result.isCacheCoherent()) {
        extensiveTests(*this);
        extensiveTests(result);
    }
    assert(result.isCacheCoherent());
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
            if (currentPos == blackKingStartPos && (figures[OwnRook] & 0b10000000ul) &&
                (figures[None] & castling1Fields) == castling1Fields && !isThreatened(blackKingStartPos) &&
                !isThreatened(blackKingStartPos << 1) && !isThreatened(blackKingStartPos << 2)) {
                checkedMove<EE, 1>(currentPos, OwnKing, OwnKing, func);
            }
            else if (
                currentPos == whiteKingStartPos && (figures[OwnRook] & 0b10000000ul << 56) &&
                (figures[None] & castling3Fields) == castling3Fields && !isThreatened(whiteKingStartPos) &&
                !isThreatened(whiteKingStartPos << 1) && !isThreatened(whiteKingStartPos << 2)) {
                checkedMove<EE, 1>(currentPos, OwnKing, OwnKing, func);
            }
        }
        if (castling[1]) {
            if (currentPos == blackKingStartPos && (figures[OwnRook] & 0b00000001ul) &&
                (figures[None] & castling2Fields) == castling2Fields && !isThreatened(blackKingStartPos) &&
                !isThreatened(blackKingStartPos >> 1) && !isThreatened(blackKingStartPos >> 2) &&
                !isThreatened(blackKingStartPos >> 3)) {
                checkedMove<WWW, 1>(currentPos, OwnKing, OwnKing, func);
            }
            else if (
                currentPos == whiteKingStartPos && (figures[OwnRook] & 0b00000001ul << 56) &&
                (figures[None] & castling4Fields) == castling4Fields && !isThreatened(whiteKingStartPos) &&
                !isThreatened(whiteKingStartPos >> 1) && !isThreatened(whiteKingStartPos >> 2) &&
                !isThreatened(whiteKingStartPos >> 3)) {
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
            if (figures[OwnFigure] & m.moveTo) {
                assert(false && "Memory error.");
            }
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
            if (figures[OwnFigure] & m.moveTo) {
                assert(false && "Memory error.");
            }
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
            if (::isValidMove<fig>(from, to, figures[AnyFigure])) {
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
