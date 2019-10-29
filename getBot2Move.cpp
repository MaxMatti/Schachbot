#include "boardWrapper.hpp"
#include "bot2.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <variant>

using Parameters = std::pair<BoardWrapper, std::variant<std::size_t, std::chrono::steady_clock::duration>>;

auto getDurationFromString(std::string arg) -> std::chrono::steady_clock::duration {
    if (arg.ends_with("ns")) {
        return std::chrono::nanoseconds(std::stoi(arg.substr(0, arg.length() - 2)));
    }
    else if (arg.ends_with("us") || arg.ends_with("µs")) {
        return std::chrono::microseconds(std::stoi(arg.substr(0, arg.length() - 2)));
    }
    else if (arg.ends_with("ms")) {
        return std::chrono::milliseconds(std::stoi(arg.substr(0, arg.length() - 2)));
    }
    else if (arg.ends_with("s")) {
        return std::chrono::seconds(std::stoi(arg.substr(0, arg.length() - 1)));
    }
    else if (arg.ends_with("m")) {
        return std::chrono::minutes(std::stoi(arg.substr(0, arg.length() - 1)));
    }
    else if (arg.ends_with("min")) {
        return std::chrono::minutes(std::stoi(arg.substr(0, arg.length() - 3)));
    }
    else if (arg.ends_with("h")) {
        return std::chrono::hours(std::stoi(arg.substr(0, arg.length() - 1)));
    }
    else {
        return std::chrono::nanoseconds{0};
    }
}

auto parseArgument(std::string arg, std::string start, int& i, int argc, char const* argv[]) -> std::string {
    if (arg.length() > start.length()) {
        if (arg[start.length()] == '=') {
            return arg.substr(start.length() + 1);
        }
        else {
            return arg.substr(start.length());
        }
    }
    else if (i + 1 < argc) {
        ++i;
        return argv[i];
    }
    return "";
}

auto parseParameters(int argc, char const* argv[]) -> Parameters {
    bool parameterMode = true;
    std::array<bool, 4> castling{true, true, true, true};
    bool userDefinedCastling = false;
    bool userDefinedParty = false;
    Parameters result{Board<true>{"rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR"}, std::chrono::milliseconds{200}};

    for (int i = 1; i < argc; ++i) {
        std::string arg{argv[i]};
        if (parameterMode) {
            if (arg == "--") {
                parameterMode = false;
            }
            else if (arg.starts_with("-t")) {
                result.second = getDurationFromString(parseArgument(arg, "-t", i, argc, argv));
                if (std::get<std::chrono::steady_clock::duration>(result.second) < std::chrono::seconds{1}) {
                    std::cerr << "Warning: Minimum depth is 4, time might be exceeded.\n";
                }
            }
            else if (arg.starts_with("--time")) {
                result.second = getDurationFromString(parseArgument(arg, "--time", i, argc, argv));
                if (std::get<std::chrono::steady_clock::duration>(result.second) < std::chrono::seconds{1}) {
                    std::cerr << "Warning: Minimum depth is 4, time might be exceeded.\n";
                }
            }
            else if (arg.starts_with("-d")) {
                std::size_t tmp = std::stoll(parseArgument(arg, "-d", i, argc, argv));
                if (tmp < 4ul || tmp > 20ul) {
                    std::cerr << "Error: depth must be between 4 and 20.\n";
                    exit(1);
                }
                else {
                    result.second = tmp;
                }
            }
            else if (arg.starts_with("--depth")) {
                std::size_t tmp = std::stoll(parseArgument(arg, "--depth", i, argc, argv));
                if (tmp < 4ul || tmp > 20ul) {
                    std::cerr << "Error: depth must be between 4 and 20.\n";
                    exit(1);
                }
                else {
                    result.second = tmp;
                }
            }
            else if (arg.starts_with("-c")) {
                auto castlingString = parseArgument(arg, "-c", i, argc, argv);
                if (castlingString.size() < castling.size()) {
                    continue;
                }
                for (std::size_t j = 0; j < castling.size(); ++j) {
                    castling[j] = castlingString[j] != '0';
                }
                userDefinedCastling = true;
            }
            else if (arg.starts_with("--castling")) {
                auto castlingString = parseArgument(arg, "--castling", i, argc, argv);
                if (castlingString.size() < castling.size()) {
                    continue;
                }
                for (std::size_t j = 0; j < castling.size(); ++j) {
                    castling[j] = castlingString[j] != '0';
                }
                userDefinedCastling = true;
            }
            else if (arg.starts_with("-w") || arg.starts_with("--white") || arg.starts_with("--play-white")) {
                result.first.amIWhite = true;
                userDefinedParty = true;
            }
            else if (arg.starts_with("-b") || arg.starts_with("--black") || arg.starts_with("--play-black")) {
                result.first.amIWhite = false;
                userDefinedParty = true;
            }
            else {
                result.first.whiteBoard = Board<true>{arg};
                result.first.blackBoard = result.first.whiteBoard;
            }
        }
        else {
            result.first.whiteBoard = Board<true>{arg};
            result.first.blackBoard = result.first.whiteBoard;
        }
    }
    if (userDefinedCastling) {
        result.first.whiteBoard.castling = castling;
        result.first.blackBoard.castling = castling;
    }
    return result;
}

int main(int argc, char const* argv[]) {
    auto&& [currentSituation, threshold] = parseParameters(argc, argv);
    Bot currentBot;
    // Note: can't use visit here because I can't capture structured bindings in a lambda function.
    if (std::holds_alternative<std::size_t>(threshold)) {
        auto depth = std::get<std::size_t>(threshold);
#define MOVE(d)                                                                                                        \
    if (depth == d##ul) {                                                                                              \
        std::cout << currentBot.getMove<d##ul, false>(currentSituation) << "\n";                                       \
    }
        MOVE(4)
        MOVE(5)
        MOVE(6)
        MOVE(7)
        MOVE(8)
        MOVE(9)
        MOVE(10)
        MOVE(11)
        MOVE(12)
        MOVE(13)
        MOVE(14)
        MOVE(15)
        MOVE(16)
        MOVE(17)
        MOVE(18)
        MOVE(19)
        MOVE(20)
#undef MOVE
    }
    else if (std::holds_alternative<std::chrono::steady_clock::duration>(threshold)) {
        auto timeThreshold = std::get<std::chrono::steady_clock::duration>(threshold);
        auto start = std::chrono::steady_clock::now();
        auto chosenMove = currentBot.getMove<4, false>(currentSituation);
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<6, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<8, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<10, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<12, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<14, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<16, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<18, false>(currentSituation);
        }
        if (std::chrono::steady_clock::now() - start < timeThreshold) {
            chosenMove = currentBot.getMove<20, false>(currentSituation);
        }
        std::cout << chosenMove << "\n";
    }
    return 0;
}
