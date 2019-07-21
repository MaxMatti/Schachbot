import qbs

CppApplication {
    consoleApplication: true
    cpp.cxxLanguageVersion: "c++17"
    cpp.optimization: "fast"
    cpp.cxxStandardLibrary: "libc++"
    cpp.staticLibraries: ["c++", "c++abi"]
    cpp.cxxFlags: base.concat(
        "-pthread", // threading capabilities
        "--pedantic", // best C++17 compatibilty
        "-Wall", "-Wextra", // enable more warnings
        "-ftemplate-backtrace-limit=0" // do not cut template backtraces
    )
    cpp.linkerFlags: base.concat("-lpthread")
    //cpp.driverFlags: base.concat("-fsanitize=address,undefined")
    files: [
        "board.hpp",
        "bot.cpp",
        "bot.hpp",
        "main.cpp",
        "move.cpp",
        "move.hpp",
        "piece.cpp",
        "piece.hpp",
        "tournament.cpp",
        "tournament.hpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
