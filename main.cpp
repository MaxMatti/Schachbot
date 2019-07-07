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

void signal_handler(int signal [[maybe_unused]]) { exit(0); }

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
    vector<size_t> lengths(lines.size());
    transform(lines.begin(), lines.end(), lengths.begin(), [](const vector<string>& obj) {
        return accumulate(obj.begin(), obj.end(), 0, [](size_t length, const string& line) {
            bool counting = true;
            // count all characters except those that represent terminal style information
            return max<size_t>(length, count_if(line.begin(), line.end(), [&](char it) {
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
            std::size_t size = count_if(lines[j][i].begin(), lines[j][i].end(), [&](char it) {
                if (it == '\033') {
                    counting = false;
                }
                else if (it == 'm') {
                    counting = true;
                    return false;
                }
                return counting;
            });
            stream << setw(lengths[j] + lines[j][i].length() - size + 1) << lines[j][i];
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
    bot.resetStats();
    if constexpr (loud) {
        std::vector<std::string> objs;
        justificateMove<depth>(bot, board, std::back_inserter(objs));
        std::cout << "Justification:\n" << objs;
    }
    std::flush(std::cout);
    return chosenMove;
}

int main(int argc, char const* argv[]) {
    std::signal(SIGINT, signal_handler);
    if (argc < 1) {
        std::cout << *argv << "\n";
    }
    std::array<int, 128> initValues = {0};
    initValues[None] = 0;
    initValues[OwnKing] = 1000;
    initValues[OwnQueen] = 9;
    initValues[OwnRook] = 5;
    initValues[OwnBishop] = 3;
    initValues[OwnKnight] = 3;
    initValues[OwnPawn] = 1;
    initValues[OwnFigure] = 1;
    initValues[EnemyKing] = -initValues[OwnKing];
    initValues[EnemyQueen] = -initValues[OwnQueen];
    initValues[EnemyRook] = -initValues[OwnRook];
    initValues[EnemyBishop] = -initValues[OwnBishop];
    initValues[EnemyKnight] = -initValues[OwnKnight];
    initValues[EnemyPawn] = -initValues[OwnPawn];
    initValues[EnemyFigure] = -initValues[OwnFigure];
    /*std::string initBoard = "R       "
                                                    "        "
                                                    "        "
                                                    "  k     "
                                                    "        "
                                                    "     K  "
                                                    "        "
                                                    "        ";*/
    std::string initBoard =
        "rnbqkbnr"
        "pppppppp"
        "        "
        "        "
        "        "
        "        "
        "PPPPPPPP"
        "RNBQKBNR";
    /*std::string initBoard = " k  n  r"
                                                    " pp   p "
                                                    "p  rN qp"
                                                    "Q    p  "
                                                    "   p    "
                                                    "     P P"
                                                    "PPP  PP "
                                                    " KR R   ";*/
    Board<true> currentSituation(initBoard);
    Board<false> otherSituation;
    Bot currentBot;
    while (currentSituation.isValid()) {
        std::cout << currentSituation;
        Move move;
        move = getInputMove(currentSituation);
        if (move == Move() || !currentSituation.isValidMove(move)) {
            break;
        }
        otherSituation = currentSituation.applyMove(move);
        std::cout << otherSituation;

        /*std::vector<Move> validMoves;
        otherSituation.forEachValidMove([&](auto i) {
            if (!otherSituation.isValidMove(i)) {
                std::cout << "invalid ";
            }
            std::cout << "move: " << i << "(" << otherSituation.at(i.moveFrom) << "->" << otherSituation.at(i.moveTo)
                      << ")" << std::endl;
            validMoves.push_back(i);
        });*/
        // auto chosenMove = validMoves.at(std::rand() % validMoves.size());
        auto start = std::chrono::steady_clock::now();
        auto chosenMove = getMove<2, true>(currentBot, otherSituation);
        using namespace std::chrono_literals;
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<4, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<6, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<8, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<10, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<12, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<14, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<16, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<18, true>(currentBot, otherSituation);
        }
        if (std::chrono::steady_clock::now() - start < 1s) {
            chosenMove = getMove<20, true>(currentBot, otherSituation);
        }
        std::flush(std::cout);
        currentSituation = otherSituation.applyMove(chosenMove);
    }
    currentSituation.forEachValidMove([&](auto i) {
        auto tmp = currentSituation.applyMove(i);
        std::cout << i << (tmp.isValid() ? "   (valid)\n" : " (invalid)\n");
    });
    std::cout << currentSituation;
    return 0;
}
