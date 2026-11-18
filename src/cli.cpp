#include "cli.hpp"
#include "cfgen.hpp"
#include "compiler.hpp"
#include "project.hpp"

#include <iostream>
#include <string>

namespace {

bool IncludeFramework = false;

bool IsFrameworkFlag(const std::string& argument) {
    return argument == "-framework" || argument == "-f";
}

}  // namespace

// Shows all the commands and what they do
void PrintCommandUsage() {
    std::cout
        << "\033[1m" << "Basic commands:" << "\033[0m" << "\n"
        << "    -h | -help              > Will show this output." << "\n"
        << "    -v | -version | -ver    > Will show the project version and branch." << "\n"
        << "    -u | -usage             > Will show how to use all commands." << "\n"
        << "    -c | -compile FILE      > Compiles a scene .json into a .vrtx file." << "\n"
        << "    -o | -output FILE       > Sets where the .vrtx gets written (default is the scene's name)." << "\n"
        << "    init                    > Creates the source folders (server, client, shared)." << "\n"
        << "    config                  > Creates the config folder (project name, version etc.)." << "\n"
        << "    build [-framework|-f]   > Compiles scripts into a .vrtx file; optionally embeds Velocity Framework." << "\n";
}

// ARGUMENT STUFF
// Handles all commands which require parameters like if you had this
// `-name NullPointer`

// This is just a test
void HandleTest(std::string Value) {
    std::cout << Value << "\n";
}

// Sets the scene which gets compiled
void HandleCompile(std::string Value) {
    SetCompileInput(Value);
}

// Sets where the compiled .vrtx gets written
void HandleOutput(std::string Value) {
    SetCompileOutput(Value);
}

// All the flags (should be configurable)
ValueFlag ValueFlags[] = {
    {
        "-test", 
        HandleTest
    },
    {
        "-c", 
        HandleCompile
    },
    {
        "-compile", 
        HandleCompile
    },
    {
        "-o", 
        HandleOutput
    },
    {
        "-output", 
        HandleOutput
    },
};

const int ValueFlagCount = sizeof(ValueFlags) / sizeof(ValueFlags[0]);

// Finds a flag from a argument
ValueFlag* FindValueFlag(std::string Argument) {
    for (int index = 0; index < ValueFlagCount; index++) {
        if (ValueFlags[index].Name == Argument) {
            return &ValueFlags[index];
        }
    }
    return nullptr;
}

// Just check if a flag needs a value after it
bool FlagNeedsValue(std::string Argument) {
    return FindValueFlag(Argument) != nullptr;
}

// STANDALONE STUFF
// Handles all standalone commands
ArgumentResult HandleSingleArgument(std::string CurrentArgument) {
    // If the currentArgument doesn't exist then return.
    if (CurrentArgument == "") {
        return ArgumentResult::Ok;
    }

    // Give all help details
    if (CurrentArgument == "-h" || CurrentArgument == "-help") {
        std::cout 
            << "\033[1m" << "Project details" << "\033[0m" << "\n"
            << "    Project version: " << GetCommit() << "\n"
            << "    From branch: " << GetBranch() << "\n"
            << "\n"
            << "\033[1m" << "Velocity" << "\033[0m" << "\n"
            << "    Velocity is a compiler, turns all of your `.luau.server / .luau.client / .luau` into a .vrtx file." << "\n"
            << "    .vrtx is the file extension used by Vortex. Similar to Roblox's .rbxl." << "\n"
            << "\n";
        PrintCommandUsage();

        return ArgumentResult::Ok;
    }

    // All version data
    else if (CurrentArgument == "-v" || CurrentArgument == "-version" || CurrentArgument == "-ver") {
        std::cout 
            << "\033[1m" << "Velocity" << "\033[0m" << "\n"
            << "    Project version: " << GetCommit() << "\n"
            << "    From branch: " << GetBranch() << "\n";

        return ArgumentResult::Ok;
    }

    // All usage data
    else if (CurrentArgument == "-u" || CurrentArgument == "-usage") {
        PrintCommandUsage();

        return ArgumentResult::Ok;
    }

    // Creates the source folders
    else if (CurrentArgument == "init") {
        if (!RunInit()) {
            return ArgumentResult::CommandFailed;
        }

        return ArgumentResult::Ok;
    }

    // Creates the config folder
    else if (CurrentArgument == "config") {
        if (!RunConfig()) {
            return ArgumentResult::CommandFailed;
        }

        return ArgumentResult::Ok;
    }

    // Compiles the scripts into a .vrtx file
    else if (CurrentArgument == "build") {
        if (!RunBuild(IncludeFramework)) {
            return ArgumentResult::CommandFailed;
        }

        return ArgumentResult::Ok;
    }

    // No such instruction.
    return ArgumentResult::InvalidArgument;
}

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]) {
    std::string currentInstruction = "";
    std::string previousInstruction = "";
    IncludeFramework = false;

    for (int currentIndex = 1; currentIndex < argc; currentIndex++) {
        if (IsFrameworkFlag(argv[currentIndex])) {
            IncludeFramework = true;
        }
    }

    for (int currentIndex = 1; currentIndex < argc; currentIndex++) {
        // Assign current instruction
        currentInstruction = argv[currentIndex];

        if (IsFrameworkFlag(currentInstruction)) {
            continue;
        }

        ValueFlag* valueFlag = FindValueFlag(currentInstruction);
        if (valueFlag != nullptr) {
            if (currentIndex + 1 >= argc) {
                std::cout 
                << "\033[1;31m" 
                << "ERROR: "
                << "\033[0m"
                << currentInstruction << " needs a value." << "\n";

                return ArgumentResult::InvalidArgument;
            }

            valueFlag->Handler(argv[currentIndex + 1]);
            currentIndex++;
            continue;
        }

        // Handle the instruction
        ArgumentResult result = HandleSingleArgument(currentInstruction);
        if (result != ArgumentResult::Ok) {
            // Red and bold and cool "ERROR: " print
            std::cout 
            << "\033[1;31m" 
            << "ERROR: "
            << "\033[0m";

            // For all invalid arguments
            if (result == ArgumentResult::InvalidArgument) {
                std::cout << "ArgumentResult::InvalidArgument, please provide proper arguments." << "\n";
            }

            return result;
        }

        // Assign next instruction
        previousInstruction = argv[currentIndex];
    }

    // Every argument has been read, so now compile if a scene was given
    if (!RunCompile()) {
        return ArgumentResult::CompileFailed;
    }

    return ArgumentResult::Ok;
}