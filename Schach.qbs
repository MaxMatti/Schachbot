import qbs

Project {
    name: "Schach"

    Application {
        Depends { name: "cpp" }

        consoleApplication: true
        cpp.cxxLanguageVersion: "c++17"
        cpp.optimization: "fast"
        cpp.cxxStandardLibrary: "libc++"
        cpp.staticLibraries: ["c++", "c++abi"]
        cpp.cxxFlags: [
            "-pthread", // threading capabilities
            "--pedantic", // best C++17 compatibilty
            "-Wall", "-Wextra", // enable more warnings
            "-ftemplate-backtrace-limit=0", // do not cut template backtraces
            "-fno-omit-frame-pointer", // leave frame pointer in for perf
            "-ffast-math", // allow rearranging floating point operations
            "-O3" // optimization
        ]
        cpp.linkerFlags: base.concat("-lpthread")
        //cpp.driverFlags: base.concat("-fsanitize=address,undefined")

        Group {
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }
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
    }
}
