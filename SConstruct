import os

mainEnv = Environment(
    CC="clang",
    CXX="clang++",
    CXXFLAGS="-std=c++2a -stdlib=libc++ -flto -Wall -Wextra -O3 -g -mtune=native -march=native -fno-omit-frame-pointer -ftemplate-backtrace-limit=0",
    LINKFLAGS="-stdlib=libc++ -flto")
mainEnv['ENV']['TERM'] = os.environ['TERM']

fastEnv = Environment(
    CC="clang",
    CXX="clang++",
    CXXFLAGS="-std=c++2a -stdlib=libc++ -flto -Wall -Wextra -O3 -g -mtune=skylake -march=skylake -fno-omit-frame-pointer -ftemplate-backtrace-limit=0",
    LINKFLAGS="-stdlib=libc++ -flto -static",
    LIBS=["c++abi", "pthread"])
fastEnv['ENV']['TERM'] = os.environ['TERM']

testEnv = Environment(
    CC="clang",
    CXX="clang++",
    CXXFLAGS="-std=c++2a -stdlib=libc++ -Wall -Wextra -O3 -g -fno-omit-frame-pointer -ftemplate-backtrace-limit=0",
    LINKFLAGS="-stdlib=libc++",
    CPPPATH=["/usr/include/gtest"],
    LIBS=["gtest_main", "gtest"],
    LIBPATH=["/usr/include/gtest"])
testEnv['ENV']['TERM'] = os.environ['TERM']

#testEnv.Program(target="gtest", source=["board.test.cpp", "move.test.cpp"])
#mainEnv.Program(target="main", source=["main.cpp", "bot.cpp", "move.cpp", "piece.cpp"])
fastEnv.Program(target="main-uni", source=["main.cpp", "bot.cpp", "move.cpp", "piece.cpp"])
#mainEnv.Program(target="getBotMove", source=["getBotMove.cpp", "bot.cpp", "move.cpp", "piece.cpp"])
#mainEnv.Program(target="getBot1Move", source=["getBot1Move.cpp", "bot1.cpp", "move.cpp", "piece.cpp"])
#mainEnv.Program(target="getBot2Move", source=["getBot2Move.cpp", "bot2.cpp", "move.cpp", "piece.cpp"])
