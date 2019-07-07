#include "bot.hpp"
#include "move.hpp"

Bot::Bot() {
    values[OwnKing] = 1;
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
}

Bot::Bot(const Bot& previous, const float& mutationIntensity, std::mt19937& generator)
    : values(previous.values) {
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    for (int& i : values) {
        if (distribution(generator) < mutationIntensity) {
            i += 1;
        }
        else if (distribution(generator) < mutationIntensity) {
            i -= 1;
        }
    }
}

std::string Bot::printStats() const {
    std::ostringstream result;
    result << statistics.size();
    auto width = result.str().size();
    result.clear();
    for (std::size_t i = 0; i < statistics.size(); ++i) {
        result << std::setw(width) << i << ": " << statistics[i] << "\n";
    }
    return result.str();
}

void Bot::resetStats() { statistics.resize(0); }

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

bool operator<(const Bot& bot1, const Bot& bot2) { return bot1.values < bot2.values; }

bool operator==(const Bot& bot1, const Bot& bot2) { return bot1.values == bot2.values; }
