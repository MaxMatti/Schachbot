#include "bot2.hpp"
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
    values[OwnKing] = 1000;
    values[OwnQueen] = 9;
    values[OwnRook] = 5;
    values[OwnBishop] = 3;
    values[OwnKnight] = 3;
    values[OwnPawn] = 1;
    values[OwnFigure] = 1000;
    values[EnemyKing] = -values[OwnKing];
    values[EnemyQueen] = -values[OwnQueen];
    values[EnemyRook] = -values[OwnRook];
    values[EnemyBishop] = -values[OwnBishop];
    values[EnemyKnight] = -values[OwnKnight];
    values[EnemyPawn] = -values[OwnPawn];
    values[EnemyFigure] = values[OwnFigure];
    strengths[None] = 0;
    strengths[AnyFigure] = 0;
    strengths[OwnKing] = 1;
    strengths[OwnQueen] = 9;
    strengths[OwnRook] = 5;
    strengths[OwnBishop] = 3;
    strengths[OwnKnight] = 3;
    strengths[OwnPawn] = 1;
    strengths[OwnFigure] = 1;
    strengths[EnemyKing] = -strengths[OwnKing];
    strengths[EnemyQueen] = -strengths[OwnQueen];
    strengths[EnemyRook] = -strengths[OwnRook];
    strengths[EnemyBishop] = -strengths[OwnBishop];
    strengths[EnemyKnight] = -strengths[OwnKnight];
    strengths[EnemyPawn] = -strengths[OwnPawn];
    strengths[EnemyFigure] = strengths[OwnFigure];
    weaknesses[None] = 0;
    weaknesses[AnyFigure] = 0;
    weaknesses[OwnKing] = 1000;
    weaknesses[OwnQueen] = 9;
    weaknesses[OwnRook] = 5;
    weaknesses[OwnBishop] = 3;
    weaknesses[OwnKnight] = 3;
    weaknesses[OwnPawn] = 1;
    weaknesses[OwnFigure] = 1;
    weaknesses[EnemyKing] = -weaknesses[OwnKing];
    weaknesses[EnemyQueen] = -weaknesses[OwnQueen];
    weaknesses[EnemyRook] = -weaknesses[OwnRook];
    weaknesses[EnemyBishop] = -weaknesses[OwnBishop];
    weaknesses[EnemyKnight] = -weaknesses[OwnKnight];
    weaknesses[EnemyPawn] = -weaknesses[OwnPawn];
    weaknesses[EnemyFigure] = weaknesses[OwnFigure];
}

Bot::Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator)
    : values(previous.values) {
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    for (auto& i : values) {
        if (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        else if (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
    for (auto& i : strengths) {
        if (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        else if (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const Bot& bot) {
    stream << "Bot(";
    for (std::size_t i = 0; i < bot.values.size(); ++i) {
        if (getChessSymbol(static_cast<piece>(i)) != std::string{"x"}) {
            stream << static_cast<piece>(i) << ": " << bot.values[i] << ", ";
        }
    }
    stream << ")";
    return stream;
}

bool operator<(const Bot& bot1, const Bot& bot2) {
    return bot1.values < bot2.values || (bot1.values == bot2.values && bot1.strengths < bot2.strengths);
}

bool operator==(const Bot& bot1, const Bot& bot2) {
    return bot1.values == bot2.values && bot1.strengths == bot2.strengths;
}
