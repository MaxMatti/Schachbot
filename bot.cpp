#include "bot.hpp"
#include "move.hpp"

std::string printDuration(duration time) {
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(time).count();
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(time).count();
    if (sec > 1000) {
        return std::to_string(sec) + "s";
    }
    else if (millis > 1000) {
        return std::to_string(millis) + "ms";
    }
    else if (micros > 1000) {
        return std::to_string(micros) + "us";
    }
    else {
        return std::to_string(nanos) + "ns";
    }
}

std::string printDurationSince(time_point start) { return printDuration(std::chrono::steady_clock::now() - start); }

Bot::Bot() {
    values[None] = 0;
    values[AnyFigure] = 0;
    values[WhiteKing] = 10000;
    values[WhiteQueen] = 900;
    values[WhiteRook] = 500;
    values[WhiteBishop] = 300;
    values[WhiteKnight] = 300;
    values[WhitePawn] = 100;
    values[WhiteFigure] = 10000;
    values[BlackKing] = -values[WhiteKing];
    values[BlackQueen] = -values[WhiteQueen];
    values[BlackRook] = -values[WhiteRook];
    values[BlackBishop] = -values[WhiteBishop];
    values[BlackKnight] = -values[WhiteKnight];
    values[BlackPawn] = -values[WhitePawn];
    values[BlackFigure] = values[WhiteFigure];
    strengths[None] = 0;
    strengths[AnyFigure] = 0;
    strengths[WhiteKing] = 10;
    strengths[WhiteQueen] = 90;
    strengths[WhiteRook] = 50;
    strengths[WhiteBishop] = 30;
    strengths[WhiteKnight] = 30;
    strengths[WhitePawn] = 10;
    strengths[WhiteFigure] = 10;
    strengths[BlackKing] = -strengths[WhiteKing];
    strengths[BlackQueen] = -strengths[WhiteQueen];
    strengths[BlackRook] = -strengths[WhiteRook];
    strengths[BlackBishop] = -strengths[WhiteBishop];
    strengths[BlackKnight] = -strengths[WhiteKnight];
    strengths[BlackPawn] = -strengths[WhitePawn];
    strengths[BlackFigure] = strengths[WhiteFigure];
    weaknesses[None] = 0;
    weaknesses[AnyFigure] = 0;
    weaknesses[WhiteKing] = 100000;
    weaknesses[WhiteQueen] = 90;
    weaknesses[WhiteRook] = 50;
    weaknesses[WhiteBishop] = 30;
    weaknesses[WhiteKnight] = 30;
    weaknesses[WhitePawn] = 10;
    weaknesses[WhiteFigure] = 10;
    weaknesses[BlackKing] = -weaknesses[WhiteKing];
    weaknesses[BlackQueen] = -weaknesses[WhiteQueen];
    weaknesses[BlackRook] = -weaknesses[WhiteRook];
    weaknesses[BlackBishop] = -weaknesses[WhiteBishop];
    weaknesses[BlackKnight] = -weaknesses[WhiteKnight];
    weaknesses[BlackPawn] = -weaknesses[WhitePawn];
    weaknesses[BlackFigure] = weaknesses[WhiteFigure];
}

Bot::Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator)
    : values(previous.values)
    , strengths(previous.strengths)
    , weaknesses(previous.weaknesses) {
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    for (auto& i : values) {
        while (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        while (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
    for (auto& i : strengths) {
        while (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        while (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
    for (auto& i : weaknesses) {
        while (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        while (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const Bot& bot) {
    stream << "Bot(";
    for (piece i : {WhiteKing,
                    WhiteQueen,
                    WhiteRook,
                    WhiteBishop,
                    WhiteKnight,
                    WhitePawn,
                    WhiteFigure,
                    BlackKing,
                    BlackQueen,
                    BlackRook,
                    BlackBishop,
                    BlackKnight,
                    BlackPawn,
                    BlackFigure}) {
        stream << i << bot.values[i];
    }
    stream << "/";
    for (piece i : {WhiteKing,
                    WhiteQueen,
                    WhiteRook,
                    WhiteBishop,
                    WhiteKnight,
                    WhitePawn,
                    WhiteFigure,
                    BlackKing,
                    BlackQueen,
                    BlackRook,
                    BlackBishop,
                    BlackKnight,
                    BlackPawn,
                    BlackFigure}) {
        stream << i << bot.strengths[i];
    }
    stream << "/";
    for (piece i : {WhiteKing,
                    WhiteQueen,
                    WhiteRook,
                    WhiteBishop,
                    WhiteKnight,
                    WhitePawn,
                    WhiteFigure,
                    BlackKing,
                    BlackQueen,
                    BlackRook,
                    BlackBishop,
                    BlackKnight,
                    BlackPawn,
                    BlackFigure}) {
        stream << i << bot.weaknesses[i];
    }
    stream << ")";
    return stream;
}

bool operator<(const Bot& bot1, const Bot& bot2) {
    return bot1.values < bot2.values || (bot1.values == bot2.values && bot1.strengths < bot2.strengths) ||
        (bot1.values == bot2.values && bot1.strengths == bot2.strengths && bot1.weaknesses < bot2.weaknesses);
}

bool operator!=(const Bot& bot1, const Bot& bot2) {
    return bot1.values != bot2.values || bot1.strengths != bot2.strengths || bot1.weaknesses != bot2.weaknesses;
}

bool operator==(const Bot& bot1, const Bot& bot2) {
    return bot1.values == bot2.values && bot1.strengths == bot2.strengths && bot1.weaknesses == bot2.weaknesses;
}
