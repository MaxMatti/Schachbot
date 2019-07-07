import qbs

CppApplication {
    consoleApplication: true
    cpp.cxxLanguageVersion: "c++17"
    cpp.optimization: "fast"
    cpp.cxxStandardLibrary: "libc++"
    cpp.staticLibraries: ["c++", "c++abi"]
    cpp.cxxFlags: base.concat(
        "--pedantic", // best C++17 compatibilty
        "-Wall", "-Wextra", // enable more warnings
        "-ftemplate-backtrace-limit=0", // do not cut template backtraces
        "-Wno-unknown-pragmas" // ignore unknown pragmas, TODO(mstaff): this doesn't seem to be working.
    )
    //cpp.driverFlags: ['-fsanitize=address,undefined']
    files: [
        "board.hpp",
        "bot.cpp",
        "bot.hpp",
        "main.cpp",
        "move.cpp",
        "move.hpp",
        "piece.cpp",
        "piece.hpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
