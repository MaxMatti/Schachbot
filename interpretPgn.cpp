#include "bot.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

#define PRINT(x) std::cout << std::setw(8) << typeid(decltype(x)).name() << std::setw(16) << #x << ": " << x << "\n"

inline auto getSecondsSince(time_point start) {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
}

void trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char x) {
                  return x != ' ' && x != '\r' && x != '\t' && x != '\n';
              }));
    str.erase(
        std::find_if(str.rbegin(), str.rend(), [](char x) { return x != ' ' && x != '\r' && x != '\t' && x != '\n'; })
            .base(),
        str.end());
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t pos = 0;
    for (std::size_t i = str.find(delimiter, pos); i != std::string::npos; i = str.find(delimiter, pos)) {
        result.push_back(str.substr(pos, i - pos));
        pos = i + delimiter.length();
    }
    return result;
}

std::string replace(std::string&& str, const std::string& needle, const std::string& replacement) {
    for (std::size_t pos = str.find(needle); pos != std::string::npos;
         pos = str.find(needle, pos + replacement.length())) {
        str.replace(pos, needle.length(), replacement);
    }
    return std::move(str);
}

std::string replace(const std::string& str, const std::string& needle, const std::string& replacement) {
    return replace(std::string{str}, needle, replacement);
}

Move interpretMove(BoardWrapper currentBoard, std::string move, bool verbose = false) {
    if (verbose) {
        std::cout << "\"" << move << "\" -> ";
    }
    while (move.ends_with("+") || move.ends_with("#")) {
        move = move.substr(0, move.length() - 1);
    }
    if (move.length() == 0) {
        return Move{};
    }
    else if (move == "O-O") {
        if (currentBoard.amIWhite) {
            return Move{whiteKingStartPos, castling1Target, WhiteKing, WhiteKing};
        }
        else {
            return Move{blackKingStartPos, castling3Target, BlackKing, BlackKing};
        }
    }
    else if (move == "O-O-O") {
        if (currentBoard.amIWhite) {
            return Move{whiteKingStartPos, castling2Target, WhiteKing, WhiteKing};
        }
        else {
            return Move{blackKingStartPos, castling4Target, BlackKing, BlackKing};
        }
    }
    else if (move.length() > 1) {
        if (move.ends_with("e.p.")) {
            std::cout << "En Passent - not implemented yet.\n";
            return Move{};
        }
        auto lastChar = *move.rbegin();
        auto secondLastChar = *(move.rbegin() + 1);
        Move result;
        result.turnTo = None;
        // pawn promotion
        if (secondLastChar == '=') {
            result.turnTo = getPiece(currentBoard.amIWhite ? std::toupper(lastChar) : std::tolower(lastChar));
            move = move.substr(0, move.length() - 2);
            lastChar = *move.rbegin();
            secondLastChar = *(move.rbegin() + 1);
        }
        if (move.find('x') != std::string::npos) {
            move = move.substr(0, move.find('x')) + move.substr(move.find('x') + 1);
        }
        if (move.find(':') != std::string::npos) {
            move = move.substr(0, move.find(':')) + move.substr(move.find(':') + 1);
        }
        result.turnFrom = None;
        result.moveFrom = ~0ul;
        result.moveTo = 1ul << ((secondLastChar - 'a') + 8 * ('8' - lastChar));
        std::for_each(move.rbegin() + 2, move.rend(), [&](auto ch) {
            if (std::isupper(ch)) {
                ch = currentBoard.amIWhite ? std::toupper(ch) : std::tolower(ch);
                result.turnFrom = getPiece(ch);
            }
            else if (std::islower(ch)) {
                std::uint64_t col = 1ul;
                col |= col << 8;
                col |= col << 16;
                col |= col << 32;
                result.moveFrom &= col << (ch - 'a');
            }
            else if (std::isdigit(ch)) {
                std::uint64_t row = 1ul;
                row |= row << 1;
                row |= row << 2;
                row |= row << 4;
                result.moveFrom &= row << (8 * ('8' - ch));
            }
            else {
                std::cout << "Unhandled: " << ch << "\n";
            }
        });
        bool foundMatch = false;
        if (currentBoard.amIWhite) {
            currentBoard.whiteBoard.forEachValidMove([&](auto m) {
                auto newBoard = currentBoard.whiteBoard.applyMove(m);
                if ((m.moveTo == result.moveTo) && (m.moveFrom & result.moveFrom) == m.moveFrom &&
                    (result.turnFrom == None || m.turnFrom == result.turnFrom) &&
                    (result.turnTo == None || m.turnTo == result.turnTo) &&
                    !newBoard.isThreatened(newBoard.figures[newBoard.OwnKing])) {
                    result = m;
                    foundMatch = true;
                }
            });
        }
        else {
            currentBoard.blackBoard.forEachValidMove([&](auto m) {
                auto newBoard = currentBoard.blackBoard.applyMove(m);
                if ((m.moveTo == result.moveTo) && (m.moveFrom & result.moveFrom) == m.moveFrom &&
                    (result.turnFrom == None || m.turnFrom == result.turnFrom) &&
                    (result.turnTo == None || m.turnTo == result.turnTo) &&
                    !newBoard.isThreatened(newBoard.figures[newBoard.OwnKing])) {
                    result = m;
                    foundMatch = true;
                }
            });
        }
        if (!foundMatch) {
            std::cout << "No Match: \"" << move << "\" -> " << result
                      << (currentBoard.amIWhite ? " (white)" : " (black)") << "\n"
                      << currentBoard;
        }
        else if (verbose) {
            std::cout << "\"" << move << "\" -> " << result << "\n" << currentBoard;
        }
        return result;
    }
    std::cout << "Unknown Move: \"" << move << "\"\n";
    return Move{};
}

void interpretGameString(
    std::map<std::string, std::map<Move, std::size_t>>& scores,
    std::string startBoard,
    std::string gameStr,
    int whiteMultiplier,
    int blackMultiplier,
    bool verbose = false) {
    trim(gameStr);
    if (gameStr.find("--") != std::string::npos) {
        return;
    }
    auto moves = split(gameStr, " ");
    BoardWrapper currentBoard{Board<true>{startBoard}};
    for (auto& it : moves) {
        auto pos = it.find(".");
        if (pos != std::string::npos) {
            it = it.substr(pos + 1);
        }
        if (it.starts_with("..")) {
            currentBoard.amIWhite = !currentBoard.amIWhite;
            it = it.substr(2);
        }
        auto currentMove = interpretMove(currentBoard, it, verbose);
        if (currentMove == Move{}) {
            break;
        }
        scores[currentBoard.store()][currentMove] += currentBoard.amIWhite ? whiteMultiplier : blackMultiplier;
        if (currentBoard.amIWhite) {
            currentBoard.whiteBoard = currentBoard.whiteBoard.applyMove(currentMove);
            currentBoard.blackBoard = currentBoard.whiteBoard;
        }
        else {
            currentBoard.blackBoard = currentBoard.blackBoard.applyMove(currentMove);
            currentBoard.whiteBoard = currentBoard.blackBoard;
        }
        currentBoard.amIWhite = !currentBoard.amIWhite;
    }
}

void readFile(std::map<std::string, std::map<Move, std::size_t>>& scores, std::string filename) {
    std::ifstream pgnFile(filename.c_str());
    bool counting;
    int whiteMultiplier;
    int blackMultiplier;
    std::string gameStr;
    std::string startBoard = "rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR";
    for (std::string line = ""; !pgnFile.eof(); std::getline(pgnFile, line)) {
        trim(line);
        if (line.starts_with("[") && line.ends_with("]")) {
            if (line.starts_with("[Result \"") && line.ends_with("\"]")) {
                std::string scoreStr = line.substr(9, line.length() - 11);
                if (scoreStr == "1-0") {
                    counting = true;
                    whiteMultiplier = 5;
                    blackMultiplier = 1;
                }
                else if (scoreStr == "0-1") {
                    counting = true;
                    whiteMultiplier = 1;
                    blackMultiplier = 5;
                }
                else {
                    counting = true;
                    whiteMultiplier = 2;
                    blackMultiplier = 2;
                }
            }
            else if (line.starts_with("[FEN \"") && line.ends_with("\"]")) {
                startBoard = replace(line.substr(6, line.find(" ", 6) - 6), "/", "");
            }
        }
        else if (line.length() > 0) {
            gameStr += line + " ";
        }
        else if (gameStr.length() > 0) {
            // std::cout << "Startboard: " << startBoard << "\n";
            // std::cout << "Gamestr: " << gameStr << "\n";
            interpretGameString(scores, startBoard, gameStr, whiteMultiplier, blackMultiplier);
            startBoard = "rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR";
            gameStr = "";
        }
    }
    pgnFile.close();
}

void updateCache(
    std::map<std::string, std::map<Move, std::size_t>>& scores, std::string filename, std::size_t maxDataSize) {

    std::cout << "1 " << std::flush;
    const std::size_t dataSize = std::accumulate(
        scores.begin(), scores.end(), 0ul, [](const auto& sum, const auto& it) { return sum + it.second.size(); });
    if (dataSize < maxDataSize) {
        return;
    }
    std::cout << "2 " << std::flush;
    std::size_t minSize = 0;
    std::size_t sizeCounter = 0;
    if (maxDataSize == 0) {
        minSize = std::numeric_limits<std::size_t>::max();
    }
    else {
        while (dataSize - sizeCounter > maxDataSize) {
            ++minSize;
            sizeCounter = std::count_if(
                scores.begin(), scores.end(), [&minSize](const auto& it) { return it.second.size() < minSize; });
            std::cout << "(" << minSize << "/" << (dataSize - sizeCounter) << ") ";
        }
    }
    std::cout << "3 " << std::flush;
    std::ifstream oldCacheFile(filename.c_str());
    int returnCode = std::remove(filename.c_str());
    if (returnCode != 0) {
        std::cout << "Error: Could not remove old cache file. Error: " << returnCode << std::endl;
        exit(returnCode);
    }
    std::cout << "4 " << std::flush;
    std::ofstream newCacheFile(filename.c_str());
    for (std::string line = ""; !oldCacheFile.eof(); std::getline(oldCacheFile, line)) {
        std::string currentBoard = line.substr(0, line.find(" "));
        if (scores.count(currentBoard)) {
            if (scores.at(currentBoard).size() < minSize) {
                std::vector<std::string> moves = split(line.substr(line.find(" ") + 1), " ");
                if (moves.size() % 2 != 0) {
                    std::cout << "\n" << line << "\n";
                }
                assert(moves.size() % 2 == 0);
                for (auto& it : scores.at(currentBoard)) {
                    std::ostringstream tmp;
                    tmp << it.first;
                    auto pos = std::find(moves.begin(), moves.end(), tmp.str());
                    if (pos == moves.end()) {
                        moves.push_back(tmp.str());
                        moves.push_back(std::to_string(it.second));
                    }
                    else {
                        *(pos + 1) = std::to_string(std::stoull(*(pos + 1)) + it.second);
                    }
                }
                line = currentBoard + " ";
                for (const auto& it : moves) {
                    line += it + " ";
                }
                scores.erase(currentBoard);
            }
            else {
                std::vector<std::string> moves = split(line.substr(line.find(" ") + 1), " ");
                assert(moves.size() % 2 == 0);
                for (std::size_t i = 0; i < moves.size(); i += 2) {
                    scores[currentBoard][Move{moves[i]}] += std::stoull(moves[i + 1]);
                }
                continue;
            }
        }
        if (line.length() > 0) {
            newCacheFile << line << std::endl;
        }
    }
    std::cout << "5 " << std::flush;
    for (auto it = scores.begin(); it != scores.end();) {
        if (it->second.size() < minSize) {
            newCacheFile << it->first << " ";
            for (const auto& jt : it->second) {
                newCacheFile << jt.first << " " << jt.second << " ";
            }
            newCacheFile << std::endl;
            it = scores.erase(it);
        }
        else {
            ++it;
        }
    }
    std::cout << "6 " << std::flush;
    oldCacheFile.close();
    newCacheFile.close();
}

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    std::map<std::string, std::map<Move, std::size_t>> scores;
    std::string cacheFilename = "/tmp/scoreCache.txt";
    if (argc > 1) {
        cacheFilename = argv[1];
    }
    for (int i = 2; i < argc; ++i) {
        std::cout << argv[i] << std::endl;
        readFile(scores, argv[i]);
        std::cout << "Scores: " << scores.size() << " -> " << std::flush;
        updateCache(scores, cacheFilename, (i == argc - 1 ? 0 : 1000000));
        std::cout << scores.size() << "\n";
    }
    for (auto& it : scores) {
        if (it.second.size() > 2) {
            std::cout << it.first << "(" << it.second.size() << " moves)\n";
            for (auto jt : it.second) {
                std::cout << jt.first << ": " << jt.second << "\n";
            }
        }
    }
    return 0;
}
