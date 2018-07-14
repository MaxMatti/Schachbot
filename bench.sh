#!/bin/bash
echo "icc O3"
icc -std=c++14 -O3 -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
echo "icc Ofast"
icc -std=c++14 -Ofast -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
echo "g++ O3"
g++ -std=c++14 -O3 -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
echo "g++ Ofast"
g++ -std=c++14 -Ofast -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
echo "clang++ O3"
clang++ -std=c++14 -O3 -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
echo "clang++ Ofast"
clang++ -std=c++14 -Ofast -march=native -mtune=native -Wall -Wextra -Wfatal-errors -pedantic -pthread -o main main.cpp board.cpp bot.cpp move.cpp tournament.cpp && time nice -n 20 ./main
