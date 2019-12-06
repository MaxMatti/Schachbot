#include "bot.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

std::string getCachedMove(std::string cacheFilename, std::string situation) {
    std::ifstream cacheFile(cacheFilename);
    cacheFile.seekg(0, std::ios::end);
    auto endPos = cacheFile.tellg();
    decltype(endPos) startPos = 0;
    std::string line;
    while (true) {
        cacheFile.seekg((startPos + endPos) / 2);
        std::getline(cacheFile, line);
        std::getline(cacheFile, line);
        auto comparison = line.substr(0, situation.length()).compare(situation);
        if (startPos >= endPos || startPos == (startPos + endPos) / 2 || endPos == (startPos + endPos) / 2) {
            break;
        }
        else if (comparison > 0) {
            endPos = (startPos + endPos) / 2;
        }
        else if (comparison < 0) {
            startPos = (startPos + endPos) / 2;
        }
        else {
            break;
        }
    }
    if (line.starts_with(situation)) {
        std::vector<std::pair<std::string, std::size_t>> moves;
        for (auto pos = situation.length() + 1; pos < line.length();) {
            auto lineMidPos = line.find(" ", pos) + 1;
            auto lineEndPos = line.find(" ", lineMidPos) + 1;
            moves.emplace_back(
                line.substr(pos, lineMidPos - pos - 1),
                std::stoll(line.substr(lineMidPos, lineEndPos - lineMidPos - 1)));
            pos = lineEndPos;
        }
        return std::max_element(moves.begin(), moves.end(), [](auto a, auto b) { return a.second < b.second; })->first;
    }
    else {
        return "";
    }
}

int main(int argc [[maybe_unused]], char const* argv [[maybe_unused]][]) {
    std::string cacheFilename = "/tmp/scoreCache.txt";
    if (argc > 1) {
        cacheFilename = argv[1];
    }
    bool white = false;
    if (argc > 2 && std::string(argv[2]) == "--play-white") {
        white = true;
    }
    auto chosenMove = getCachedMove(cacheFilename, argv[argc - 1]);
    if (chosenMove == "") {
        std::cout << Bot{}.getMove<4, false>(BoardWrapper{white, argv[argc - 1]}) << "\n";
    }
    else {
        std::cout << chosenMove << " .\n";
    }
    return 0;
}
