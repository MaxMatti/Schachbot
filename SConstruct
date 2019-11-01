import os

env = Environment(
    CC="clang",
    CXX="clang++",
    CXXFLAGS="-std=c++2a -stdlib=libc++ -Wall -Wextra -O3 -g -fno-omit-frame-pointer -ftemplate-backtrace-limit=0",
    LINKFLAGS="-stdlib=libc++",
    CPPPATH=["/usr/include/gtest"])

env['ENV']['TERM'] = os.environ['TERM']

common_libs = ["gtest_main", "gtest"]
env.Append( LIBS = common_libs )

env.Program(target="gtest", source=["board.test.cpp"], LIBPATH=["/usr/gtest"])
env.Program(target="main", source=["main.cpp", "bot.cpp", "move.cpp", "piece.cpp"], LIBPATH=["/usr/gtest"])
env.Program(target="getBotMove", source=["getBotMove.cpp", "bot.cpp", "move.cpp", "piece.cpp"], LIBPATH=["/usr/gtest"])
env.Program(target="getBot1Move", source=["getBot1Move.cpp", "bot1.cpp", "move.cpp", "piece.cpp"], LIBPATH=["/usr/gtest"])
env.Program(target="getBot2Move", source=["getBot2Move.cpp", "bot2.cpp", "move.cpp", "piece.cpp"], LIBPATH=["/usr/gtest"])
