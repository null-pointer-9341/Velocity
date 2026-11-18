#include "cli.hpp"
#include "cfgen.hpp"

#include <iostream>
#include <string>

// Shows all the commands and what they do
void PrintCommandUsage() {
    std::cout
        << "\033[1m" << "Basic commands:" << "\033[0m" << "\n"
        << "    -h | -help              > Will show this output." << "\n"
        << "    -v | -version | -ver    > Will show the project version and branch." << "\n"
        << "    -u | -usage             > Will show how to use all commands." << "\n";
}

// ARGUMENT STUFF
// Handles all commands which require parameters like if you had this
// `-name NullPointer`

// This is just a test
void HandleTest(std::string Value) {
    std::cout << Value << "\n";
}

// All the flags (should be configurable)
ValueFlag ValueFlags[] = {
    {
        "-test", 
        HandleTest
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

    // No such instruction.
    return ArgumentResult::InvalidArgument;
}

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]) {
    std::string currentInstruction = "";
    std::string previousInstruction = "";

    for (int currentIndex = 1; currentIndex < argc; currentIndex++) {
        // Assign current instruction
        currentInstruction = argv[currentIndex];


        ValueFlag* valueFlag = FindValueFlag(currentInstruction);
        if (valueFlag != nullptr) {
            if (currentIndex + 1 >= argc) {
                std::cout 
                << "\033[1;31m" 
                << "ERROR: "
                << "\033[0m";

                std::cout
                    << "ArgumentResult::InvalidArgument, " << currentInstruction << " requires a value." << "\n"
                    << "\033[1;32m" << "INFO: " << "\033[0m" << "Use `velocity -help` to get a list of commands." << "\n";

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
                std::cout
                    << "ArgumentResult::InvalidArgument, please provide proper arguments." << "\n"
                    << "\033[1;32m" << "INFO: " << "\033[0m" << "Use `velocity -help` to get a list of commands." << "\n";
            }

            return result;
        }

        // Assign next instruction
        previousInstruction = argv[currentIndex];
    }

    return ArgumentResult::Ok;
}