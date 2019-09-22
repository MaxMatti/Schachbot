#include "board.hpp"
#include "bot.hpp"
#include "move.hpp"
#include <chrono>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>

static std::chrono::steady_clock::time_point totalStart;
static std::int64_t totalMoves;

std::string read_board() {
    std::string result;
    std::string tmp;
    do {
        std::getline(std::cin, tmp);
        if (tmp.empty() && result.empty()) {
            break;
        }
        while (tmp.length() < 8) {
            tmp += " ";
        }
        result += tmp;
    } while (result.length() < 64);
    return result.substr(0, 64);
}

std::string read_move() {
    std::string result;
    std::cin >> result;
    return result;
}

[[noreturn]] void signal_handler(int signal [[maybe_unused]]) {
    std::cout << "Evaluated " << totalMoves << " Moves (" << (totalMoves / getMsSince(totalStart)) << "/ms).\n";
    exit(0);
}

template <bool amIWhite>
Move getInputMove(Board<amIWhite> currentSituation) {
    std::string tmp;
    while (tmp.length() != 4 && tmp.length() != 5) {
        std::cin >> tmp;
    }
    std::uint64_t from = 1ul << (63 - (7 - (tmp[0] - (tmp[0] < 'I' ? 'A' : 'a')) + (tmp[1] - '1') * 8));
    std::uint64_t to = 1ul << (63 - (7 - (tmp[2] - (tmp[2] < 'I' ? 'A' : 'a')) + (tmp[3] - '1') * 8));
    piece turnTo = tmp.length() > 4 ? getPiece(tmp[4]) : None;
    return Move{from, to, currentSituation.at(from), turnTo};
}

namespace std {
ostream& operator<<(ostream& stream, const vector<string>& objects) {
    vector<vector<string>> lines;
    for (const auto& it : objects) {
        istringstream str(it);
        string currentLine;
        auto& currentObj = lines.emplace_back();
        while (getline(str, currentLine, '\n')) {
            currentObj.emplace_back(move(currentLine));
        }
    }
    size_t height = accumulate(lines.begin(), lines.end(), 0ul, [](size_t height, const vector<string>& line) {
        return max(height, line.size());
    });
    vector<long> lengths(lines.size());
    transform(lines.begin(), lines.end(), lengths.begin(), [](const vector<string>& obj) {
        return accumulate(obj.begin(), obj.end(), 0, [](long length, const string& line) {
            bool counting = true;
            // count all characters except those that represent terminal style information
            return max<long>(length, count_if(line.begin(), line.end(), [&](char it) {
                                 if (it == '\033') {
                                     counting = false;
                                 }
                                 else if (it == 'm') {
                                     counting = true;
                                     return false;
                                 }
                                 return counting;
                             }));
        });
    });
    for (auto& line : lines) {
        line.resize(height);
    }
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < lines.size(); ++j) {
            bool counting = true;
            // count all characters except those that represent terminal style information
            long size = count_if(lines[j][i].begin(), lines[j][i].end(), [&](char it) {
                if (it == '\033') {
                    counting = false;
                }
                else if (it == 'm') {
                    counting = true;
                    return false;
                }
                return counting;
            });
            stream << setw(static_cast<int>(lengths[j] + static_cast<long>(lines[j][i].length()) - size + 1))
                   << lines[j][i];
        }
        stream << "\n";
    }
    return stream;
}
} // namespace std

template <std::size_t depth, bool amIWhite, class P>
void justificateMove(Bot& bot, Board<amIWhite>& board, P pos) {
    auto move = bot.getMove<depth, false>(board);
    Board<!amIWhite> tmp = board.applyMove(move);
    *pos = tmp.print();
    if constexpr (depth > 1) {
        justificateMove<depth - 1>(bot, tmp, ++pos);
    }
}

template <std::size_t depth, bool loud, bool amIWhite>
Move getMove(Bot& bot, Board<amIWhite>& board) {
    std::cout << "Depth: " << depth << std::endl;
    auto chosenMove = bot.getMove<depth, loud>(board);
    // bot.resetStats();
    if constexpr (loud) {
        std::vector<std::string> objs;
        justificateMove<depth>(bot, board, std::back_inserter(objs));
        std::cout << "Justification:\n" << objs;
    }
    std::flush(std::cout);
    return chosenMove;
}

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::string initBoard =
        "rnbqkbnr"
        "pppppppp"
        "        "
        "        "
        "        "
        "        "
        "PPPPPPPP"
        "RNBQKBNR"; /*
     initBoard =
         "rn    nr"
         "   k ppp"
         "p p     "
         "    R   "
         " b pN   "
         "   b K N"
         "  p  PPP"
         "  B    R"; */
    Board<true> currentSituation(initBoard);
    Board<false> otherSituation;
    Bot currentBot;
    totalStart = std::chrono::steady_clock::now();
    totalMoves = 0;
    int sameCounter = 0;
    while (currentSituation.isValid() && sameCounter < 10) {
        std::cout << currentSituation;
        using namespace std::chrono_literals;
        auto start = std::chrono::steady_clock::now();
        totalMoves += currentBot.counter;
        currentBot.counter = 0;
        auto timeThreshold = 0.2s;
        auto chosenMove = getMove<4, false>(currentBot, currentSituation);
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<6, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<8, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<10, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<12, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<14, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<16, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<18, false>(currentBot, currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<20, false>(currentBot, currentSituation);
        }
        std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms.\n";
        std::cout << "Evaluated " << currentBot.counter << " Moves (" << currentBot.counter / (getMsSince(start) + 1)
                  << "/ms).\n";
        decltype(otherSituation) tmp1 = std::move(otherSituation);
        otherSituation = currentSituation.applyMove(chosenMove);
        if (otherSituation == tmp1) {
            ++sameCounter;
        }

        if (!otherSituation.isValid()) {
            break;
        }

        std::cout << otherSituation;
        start = std::chrono::steady_clock::now();
        totalMoves += currentBot.counter;
        currentBot.counter = 0;
        chosenMove = getMove<4, false>(currentBot, otherSituation);
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<6, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<8, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<10, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<12, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<14, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<16, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<18, false>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms." << std::endl;
            chosenMove = getMove<20, false>(currentBot, otherSituation);
        }
        std::cout << "Chose " << chosenMove << " in " << getMsSince(start) << " ms.\n";
        std::cout << "Evaluated " << currentBot.counter << " Moves (" << currentBot.counter / (getMsSince(start) + 1)
                  << "/ms).\n";
        decltype(currentSituation) tmp2 = std::move(currentSituation);
        currentSituation = otherSituation.applyMove(chosenMove);
        if (currentSituation == tmp2) {
            ++sameCounter;
        }
    }
    currentSituation.forEachValidMove([&](auto i) {
        auto tmp = currentSituation.applyMove(i);
        std::cout << i << (tmp.isValid() ? "   (valid)\n" : " (invalid)\n");
    });
    std::cout << currentSituation;
    std::cout << "Evaluated " << totalMoves << " Moves (" << totalMoves / (getMsSince(totalStart) + 1) << "/ms).\n";
    /*Bot parent;
    std::mt19937 engine;
    Tournament tournament;
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.addContestant(Bot(parent, 0.1f, engine));
    tournament.evaluate(true);*/
    signal_handler(0);
    return 0;
}
