#pragma once
#include "piece.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <utility>

constexpr const static std::uint64_t whiteKingStartPos = 0b00010000ul << 56;
constexpr const static std::uint64_t whiteQueenStartPos = 0b00001000ul << 56;
constexpr const static std::uint64_t whiteRookStartPos = 0b10000001ul << 56;
constexpr const static std::uint64_t whiteBishopStartPos = 0b00100100ul << 56;
constexpr const static std::uint64_t whiteKnightStartPos = 0b01000010ul << 56;
constexpr const static std::uint64_t whitePawnStartPos = 0b11111111ul << 48;
constexpr const static std::uint64_t whitePawnJumpPos = 0b11111111ul << 32;

constexpr const static std::uint64_t blackKingStartPos = 0b00010000ul;
constexpr const static std::uint64_t blackQueenStartPos = 0b00001000ul;
constexpr const static std::uint64_t blackRookStartPos = 0b10000001ul;
constexpr const static std::uint64_t blackBishopStartPos = 0b00100100ul;
constexpr const static std::uint64_t blackKnightStartPos = 0b01000010ul;
constexpr const static std::uint64_t blackPawnStartPos = 0b11111111ul << 8;
constexpr const static std::uint64_t blackPawnJumpPos = 0b11111111ul << 24;

constexpr const static std::uint64_t castling1RookStart = 0b10000000ul << 56;
constexpr const static std::uint64_t castling1RookTarget = 0b00100000ul << 56;
constexpr const static std::uint64_t castling1Target = 0b01000000ul << 56;
constexpr const static std::uint64_t castling1Fields = 0b01100000ul << 56;

constexpr const static std::uint64_t castling2RookStart = 0b00000001ul << 56;
constexpr const static std::uint64_t castling2RookTarget = 0b00000100ul << 56;
constexpr const static std::uint64_t castling2Target = 0b00000010ul << 56;
constexpr const static std::uint64_t castling2Fields = 0b00001110ul << 56;

constexpr const static std::uint64_t castling3RookStart = 0b10000000ul;
constexpr const static std::uint64_t castling3RookTarget = 0b00100000ul;
constexpr const static std::uint64_t castling3Target = 0b01000000ul;
constexpr const static std::uint64_t castling3Fields = 0b01100000ul;

constexpr const static std::uint64_t castling4RookStart = 0b00000001ul;
constexpr const static std::uint64_t castling4RookTarget = 0b00000100ul;
constexpr const static std::uint64_t castling4Target = 0b00000010ul;
constexpr const static std::uint64_t castling4Fields = 0b00001110ul;

enum Direction : std::uint8_t {
    North = 1,
    East = 4,
    South = 16,
    West = 64,
    N = North,
    E = East,
    S = South,
    W = West,
    NN = N + N,
    EE = E + E,
    SS = S + S,
    WW = W + W,
    NE = N + E,
    SE = S + E,
    SW = S + W,
    NW = N + W,
    NNN = NN + N,
    EEE = EE + E,
    SSS = SS + S,
    WWW = WW + W,
    NNE = N + N + E,
    ENE = E + N + E,
    ESE = E + S + E,
    SSE = S + S + E,
    SSW = S + S + W,
    WSW = W + S + W,
    WNW = W + N + W,
    NNW = N + N + W
};

constexpr Direction operator+(Direction l, Direction r) {
    return static_cast<Direction>(static_cast<std::uint8_t>(l) + static_cast<std::uint8_t>(r));
}

struct Move {
    std::uint64_t moveFrom{0};
    std::uint64_t moveTo{0};
    piece turnFrom{None};
    piece turnTo{None};

    Move()
        : moveFrom(0)
        , moveTo(0)
        , turnFrom(None)
        , turnTo(None) {}
    Move(std::uint64_t moveFrom, std::uint64_t moveTo, piece turnFrom, piece turnTo = None)
        : moveFrom(moveFrom)
        , moveTo(moveTo)
        , turnFrom(turnFrom)
        , turnTo(turnTo == None ? turnFrom : turnTo) {
        assert(
            (__builtin_popcountll(moveFrom) == 1 && __builtin_popcountll(moveTo) == 1) ||
            (__builtin_popcountll(moveFrom) == 0 && __builtin_popcountll(moveTo) == 0));
    }
};

std::ostream& operator<<(std::ostream& stream, const Move& move);

constexpr bool operator!=(const Move& l, const Move& r) {
    return l.moveFrom != r.moveFrom || l.moveTo != r.moveTo || l.turnFrom != r.turnFrom || l.turnTo != r.turnTo;
}

constexpr bool operator==(const Move& l, const Move& r) {
    return l.moveFrom == r.moveFrom && l.moveTo == r.moveTo && l.turnFrom == r.turnFrom && l.turnTo == r.turnTo;
}

constexpr std::int8_t getIntDir(Direction dir) {
    std::int8_t result = 0;
    if (dir & N) {
        result += 8;
    }
    if (dir & (N << 1)) {
        result += 16;
    }
    if (dir & E) {
        result -= 1;
    }
    if (dir & (E << 1)) {
        result -= 2;
    }
    if (dir & S) {
        result -= 8;
    }
    if (dir & (S << 1)) {
        result -= 16;
    }
    if (dir & W) {
        result += 1;
    }
    if (dir & (W << 1)) {
        result += 2;
    }
    return result;
}

constexpr bool contains(Direction haystack, Direction needle) { return haystack & (needle | (needle << 1)); }

constexpr bool containsTwice(Direction haystack, Direction needle) { return haystack & (needle << 1); }

namespace hidden {
constexpr std::uint64_t maskNone() { return -1ul; }

template <std::uint8_t distance>
constexpr std::uint64_t maskNorth() {
    if (distance >= 8) {
        return 0;
    }
    std::uint64_t result{0xff};
    result &= 0xff;
    result <<= 56;
    for (std::int_fast8_t i = 0; i < 7 - distance; ++i) {
        result |= result >> 8;
    }
    return result;
}

template <std::uint8_t distance>
constexpr std::uint64_t maskEast() {
    if (distance >= 8) {
        return 0;
    }
    std::uint64_t result{0xff};
    result >>= distance;
    result &= 0xff;
    result |= result << 8;
    result |= result << 16;
    result |= result << 32;
    return result;
}

template <std::uint8_t distance>
constexpr std::uint64_t maskSouth() {
    if (distance >= 8) {
        return 0;
    }
    std::uint64_t result{0xff};
    result &= 0xff;
    for (std::int_fast8_t i = 1; i < 8 - distance; ++i) {
        result |= result << 8;
    }
    return result;
}

template <std::uint8_t distance>
constexpr std::uint64_t maskWest() {
    if (distance >= 8) {
        return 0;
    }
    std::uint64_t result{0xff};
    result <<= distance;
    result &= 0xff;
    result |= result << 8;
    result |= result << 16;
    result |= result << 32;
    return result;
}

template <Direction dir, std::uint8_t distance>
constexpr std::uint64_t getMask() {
    auto mask = maskNone();
    if constexpr (containsTwice(dir, N)) {
        mask &= maskNorth<distance * 2>();
    }
    else if constexpr (contains(dir, N)) {
        mask &= maskNorth<distance>();
    }
    if constexpr (containsTwice(dir, E)) {
        mask &= maskEast<distance * 2>();
    }
    else if constexpr (contains(dir, E)) {
        mask &= maskEast<distance>();
    }
    if constexpr (containsTwice(dir, S)) {
        mask &= maskSouth<distance * 2>();
    }
    else if constexpr (contains(dir, S)) {
        mask &= maskSouth<distance>();
    }
    if constexpr (containsTwice(dir, W)) {
        mask &= maskWest<distance * 2>();
    }
    else if constexpr (contains(dir, W)) {
        mask &= maskWest<distance>();
    }
    return mask;
}

template <std::int8_t distance, class F, std::uint64_t mask>
bool checkedMove(std::uint64_t pos, piece turnFrom, piece turnTo, F&& func) {
    auto startPos = pos;
    pos &= mask;
    if constexpr (distance >= 64 || distance <= -64) {
        return false;
    }
    else if constexpr (distance > 0) {
        pos >>= distance;
    }
    else if constexpr (distance < 0) {
        pos <<= -distance;
    }
    return pos && func(Move(startPos, pos, turnFrom, turnTo));
}

template <std::int8_t distance, std::uint64_t mask>
bool uncheckedMove(std::uint64_t pos) {
    pos &= mask;
    if constexpr (distance >= 64 || distance <= -64) {
        return false;
    }
    else if constexpr (distance > 0) {
        pos >>= distance;
    }
    else if constexpr (distance < 0) {
        pos <<= -distance;
    }
    return pos;
}

// TODO(mstaff): Can this be written down more elegantly? Also needs to be tested.
template <Direction dir, int distance>
constexpr bool isInDirection(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles) {
    constexpr auto direction = getIntDir(dir);
    for (auto i = 1; i < distance + 1 && abs(direction * i) < 64; ++i) {
        auto pos = direction < 0 ? (from >> (-direction * i)) : (from << (direction * i));
        if (pos & obstacles) {
            return false;
        }
        if (pos & to) {
            return true;
        }
    }
    return false;
}
} // namespace hidden

template <Direction dir, std::uint8_t distance, class F>
bool checkedMove(std::uint64_t pos, piece turnFrom, piece turnTo, F&& func) {
    return hidden::checkedMove<distance * getIntDir(dir), F, hidden::getMask<dir, distance>()>(
        pos, turnFrom, turnTo, std::forward<F>(func));
}

template <Direction dir, std::uint8_t distance>
bool uncheckedMove(std::uint64_t pos) {
    return hidden::uncheckedMove<distance * getIntDir(dir), hidden::getMask<dir, distance>()>(pos);
}

template <Direction dir, std::uint8_t distance, class F>
bool checkedMoveUntil(std::uint64_t pos, piece turnFrom, piece turnTo, F&& func) {
    if constexpr (distance > 1) {
        if (!checkedMoveUntil<dir, distance - 1, F>(pos, turnFrom, turnTo, std::forward<F>(func))) {
            return false;
        }
    }
    return hidden::checkedMove<distance * getIntDir(dir), F, hidden::getMask<dir, distance>()>(
        pos, turnFrom, turnTo, std::forward<F>(func));
}

template <Direction dir, std::uint8_t distance>
bool uncheckedMoveUntil(std::uint64_t pos) {
    if constexpr (distance > 1) {
        if (!uncheckedMoveUntil<dir, distance - 1>(pos)) {
            return false;
        }
    }
    return hidden::uncheckedMove<distance * getIntDir(dir), hidden::getMask<dir, distance>()>(pos);
}

template <class F>
constexpr bool forEachPos(std::uint64_t positions, F&& func) {
    while (__builtin_popcountll(positions) != 0) {
        std::uint64_t currentPos = 1;
        currentPos <<= __builtin_ctzll(positions);
        if (!func(currentPos)) {
            return false;
        }
        positions &= ~currentPos;
    }
    return true;
}

constexpr bool isCastling(std::uint64_t from, std::uint64_t to) {
    assert(__builtin_popcountll(from) == 1);
    assert(__builtin_popcountll(to) == 1);
    return (from == whiteKingStartPos && (to == whiteKingStartPos << 2 || to == whiteKingStartPos >> 3)) ||
        (from == blackKingStartPos && (to == blackKingStartPos << 2 || to == blackKingStartPos >> 3));
}

constexpr bool isValidKingMove(std::uint64_t from, std::uint64_t to) {
    assert(__builtin_popcountll(from) == 1);
    assert(__builtin_popcountll(to) == 1);
    auto fromOffset = __builtin_ctzll(from);
    auto toOffset = __builtin_ctzll(to);
    auto diff = fromOffset > toOffset ? fromOffset - toOffset : toOffset - fromOffset;
    return diff == 0 || diff == 1 || diff == 7 || diff == 8 || diff == 9 || isCastling(from, to);
}

constexpr bool isValidQueenMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles) {
    assert(__builtin_popcountll(from) == 1);
    obstacles &= ~from;
    obstacles &= ~to;
    bool result = false;
    result |= hidden::isInDirection<N, 8>(from, to, obstacles);
    result |= hidden::isInDirection<E, 8>(from, to, obstacles);
    result |= hidden::isInDirection<S, 8>(from, to, obstacles);
    result |= hidden::isInDirection<W, 8>(from, to, obstacles);
    result |= hidden::isInDirection<NE, 8>(from, to, obstacles);
    result |= hidden::isInDirection<SE, 8>(from, to, obstacles);
    result |= hidden::isInDirection<SW, 8>(from, to, obstacles);
    result |= hidden::isInDirection<NW, 8>(from, to, obstacles);
    return result;
}

constexpr bool isValidRookMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles) {
    assert(__builtin_popcountll(from) == 1);
    obstacles &= ~from;
    obstacles &= ~to;
    bool result = false;
    result |= hidden::isInDirection<N, 8>(from, to, obstacles);
    result |= hidden::isInDirection<E, 8>(from, to, obstacles);
    result |= hidden::isInDirection<S, 8>(from, to, obstacles);
    result |= hidden::isInDirection<W, 8>(from, to, obstacles);
    return result;
}

constexpr bool isValidBishopMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles) {
    assert(__builtin_popcountll(from) == 1);
    obstacles &= ~from;
    obstacles &= ~to;
    bool result = false;
    result |= hidden::isInDirection<NE, 8>(from, to, obstacles);
    result |= hidden::isInDirection<SE, 8>(from, to, obstacles);
    result |= hidden::isInDirection<SW, 8>(from, to, obstacles);
    result |= hidden::isInDirection<NW, 8>(from, to, obstacles);
    return result;
}

constexpr bool isValidKnightMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles) {
    assert(__builtin_popcountll(from) == 1);
    obstacles &= ~from;
    obstacles &= ~to;
    bool result = false;
    result |= hidden::isInDirection<NNE, 1>(from, to, obstacles);
    result |= hidden::isInDirection<ENE, 1>(from, to, obstacles);
    result |= hidden::isInDirection<ESE, 1>(from, to, obstacles);
    result |= hidden::isInDirection<SSE, 1>(from, to, obstacles);
    result |= hidden::isInDirection<SSW, 1>(from, to, obstacles);
    result |= hidden::isInDirection<WSW, 1>(from, to, obstacles);
    result |= hidden::isInDirection<WNW, 1>(from, to, obstacles);
    result |= hidden::isInDirection<NNW, 1>(from, to, obstacles);
    return result;
}

template <bool amIWhite>
constexpr bool isInitialPawnPosition(std::uint64_t position) {
    assert(__builtin_popcountll(position) == 1);
    if constexpr (amIWhite) {
        return (0b11111111ul << 48) & position;
    }
    else {
        return (0b11111111ul << 8) & position;
    }
}

template <bool amIWhite>
constexpr bool isValidPawnMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles, std::uint64_t enPassent) {
    assert(__builtin_popcountll(from) == 1);
    obstacles &= ~from;
    bool result = false;
    constexpr Direction dir = amIWhite ? S : N;
    constexpr Direction l = dir + E;
    constexpr Direction r = dir + W;
    if (obstacles & to || enPassent & to) {
        obstacles &= ~to;
        result |= hidden::isInDirection<l, 1>(from, to, obstacles);
        result |= hidden::isInDirection<r, 1>(from, to, obstacles);
    }
    else {
        if (isInitialPawnPosition<amIWhite>(from)) {
            result |= hidden::isInDirection<dir, 2>(from, to, obstacles);
        }
        result |= hidden::isInDirection<dir, 1>(from, to, obstacles);
    }
    return result;
}

template <piece fig>
constexpr bool isValidMove(std::uint64_t from, std::uint64_t to, std::uint64_t obstacles, std::uint64_t enPassent) {
    if constexpr (isKing(fig)) {
        return isValidKingMove(from, to);
    }
    else if constexpr (isQueen(fig)) {
        return isValidQueenMove(from, to, obstacles);
    }
    else if constexpr (isRook(fig)) {
        return isValidRookMove(from, to, obstacles);
    }
    else if constexpr (isBishop(fig)) {
        return isValidBishopMove(from, to, obstacles);
    }
    else if constexpr (isKnight(fig)) {
        return isValidKnightMove(from, to, obstacles);
    }
    else if constexpr (isPawn(fig)) {
        if constexpr (fig == WhitePawn) {
            return isValidPawnMove<true>(from, to, obstacles, enPassent);
        }
        else if constexpr (fig == BlackPawn) {
            return isValidPawnMove<false>(from, to, obstacles, enPassent);
        }
    }
    assert(false && "Cannot test validity of moves for this type of piece!");
    return false;
}

bool isValidMove(Move move, std::uint64_t obstacles, std::uint64_t enPassent);
