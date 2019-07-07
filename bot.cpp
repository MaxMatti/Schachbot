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

std::string Bot::printStats(duration time) const {
    std::vector<std::vector<std::string>> table;
    std::ostringstream result;
    for (std::size_t i = 0; i < statistics.size(); ++i) {
        auto& currentLine = table.emplace_back();
        currentLine.resize(6);
        currentLine[0] = std::to_string(i);
        currentLine[1] = ": ";
        currentLine[2] = std::to_string(statistics[i]);
        currentLine[3] = " (";
        currentLine[4] = printDuration(time / statistics[i]);
        currentLine[5] = ")\n";
    }
    std::vector<std::size_t> lengths;
    lengths.resize(std::accumulate(
        table.begin(), table.end(), 0ul, [](std::size_t value, auto line) { return std::max(value, line.size()); }));
    for (std::size_t i = 0; i < lengths.size(); ++i) {
        lengths[i] = std::accumulate(table.begin(), table.end(), 0ul, [&i](std::size_t value, auto line) {
            return line.size() > i ? std::max(value, line[i].length()) : value;
        });
    }
    for (const auto& line : table) {
        for (std::size_t i = 0; i < line.size(); ++i) {
            result << std::setw(lengths[i]) << line[i];
        }
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
