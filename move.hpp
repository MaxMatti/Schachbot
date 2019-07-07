#pragma once
#include "piece.hpp"

#include <cassert>
#include <cstdint>
#include <sstream>
#include <utility>

enum Direction : std::uint8_t {
    North = 1,
    East = 4,
    South = 16,
    West = 64,
    N = North,
    E = East,
    S = South,
    W = West,
    NE = N + E,
    SE = S + E,
    SW = S + W,
    NW = N + W,
    NNE = N + N + E,
    ENE = E + N + E,
    ESE = E + S + E,
    SSE = S + S + E,
    SSW = S + S + W,
    WSW = W + S + W,
    WNW = W + N + W,
    NNW = N + N + W
};

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
bool operator!=(const Move& l, const Move& r);
bool operator==(const Move& l, const Move& r);

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
constexpr std::uint64_t maskNone() { return -1; }

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
bool move(std::uint64_t pos, piece p, F&& func) {
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
    return pos && func(Move(startPos, pos, p));
}
} // namespace hidden

template <Direction dir, std::uint8_t distance, class F>
bool move(std::uint64_t pos, piece p, F&& func) {
    return hidden::move<distance * getIntDir(dir), F, hidden::getMask<dir, distance>()>(pos, p, std::forward<F>(func));
}

template <Direction dir, std::uint8_t distance, class F>
bool moveUntil(std::uint64_t pos, piece p, F&& func) {
    if constexpr (distance > 1) {
        if (!moveUntil<dir, distance - 1, F>(pos, p, std::forward<F>(func))) {
            return false;
        }
    }
    return hidden::move<distance * getIntDir(dir), F, hidden::getMask<dir, distance>()>(pos, p, std::forward<F>(func));
}

template <bool amIWhite>
bool isInLastRow(std::uint64_t pos) {
    assert(__builtin_popcountll(pos) == 1 && "Can only check last row for one position at a time!");
    if constexpr (amIWhite) {
        return pos & 0b11111111ul << 56;
    }
    else {
        return pos & 0b11111111ul;
    }
}
