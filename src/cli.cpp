#include "cli.hpp"
#include "cfgen.hpp"

#include <iostream>

// Shows all the commands and what they do
void PrintCommandUsage() {
    std::cout
        << "\033[1m" << "Basic commands:" << "\033[0m" << "\n"
        << "    -h | -help              > Will show this output." << "\n"
        << "    -v | -version | -ver    > Will show the project version and branch." << "\n"
        << "    -u | -usage             > Will show how to use all commands." << "\n"
        << "    -c | -console           > Opens the Velocity console." << "\n";
}

// Opens up the console
int RunConsole() {

}

// Handles a single argument
ArgumentResult HandleSingleArgument(std::string CurrentArgument, std::string PreviousArgument) {
    // If the currentArgument doesn't exist then return.
    if (CurrentArgument == "") {
        return ArgumentResult::Ok;
    }

    // Check all standalone stuff (like help, version etc.)
    if (CurrentArgument == "-h" || CurrentArgument == "-help") {
        std::cout 
            << "\033[1m" << "Project details" << "\033[0m" << "\n"
            << "    Project version: " << GetCommit() << "\n"
            << "    From branch: " << GetBranch() << "\n"
            << "\n"
            << "\033[1m" << "Velocity" << "\033[0m" << "\n"
            << "    Velocity is a compiler, turns all of your `.luau.server / .luau.client / .luau` into a .vrtx file." << "\n"
            << "    .vrtx is the file extension used by Vortex. Similar to Roblox's .rbxl." << "\n"
            << "    \n"
            << "You can run the Velocity console by using `velocity -console` which will allow you to run commands without the requirement of writing `velocity` every time." << "\n";
        PrintCommandUsage();

        return ArgumentResult::Ok;
    }
    else if (CurrentArgument == "-v" || CurrentArgument == "-version" || CurrentArgument == "-ver") {
        std::cout 
            << "\033[1m" << "Velocity" << "\033[0m" << "\n"
            << "Project version: " << GetCommit() << "\n"
            << "From branch: " << GetBranch() << "\n";

        return ArgumentResult::Ok;
    }
    else if (CurrentArgument == "-u" || CurrentArgument == "-usage") {
        PrintCommandUsage();

        return ArgumentResult::Ok;
    }
    else if (CurrentArgument == "-c" || CurrentArgument == "-console") {

    }

    // Non-standalone stuff (like -name User123 (THAT IS NOT IN THE CODE AND JUST A EXAMPLE!!))
    

    // Everything passed correctly.
    return ArgumentResult::Ok;
}

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]) {
    std::string currentInstruction = "";
    std::string previousInstruction = "";

    for (int currentIndex = 1; currentIndex < argc; currentIndex++) {
        // Assign current instruction
        currentInstruction = argv[currentIndex];

        // Handle the instruction
        ArgumentResult result = HandleSingleArgument(currentInstruction, previousInstruction);
        if (result != ArgumentResult::Ok) {
            // Red and bold and cool "Error: " print
            std::cout 
            << "\033[1;31m" 
            << "Error: "
            << "\033[0m";

            // For all invalid arguments
            if (result == ArgumentResult::InvalidArgument) {
                std::cout << "ArgumentResult::InvalidArgument, please provide a proper argument.";
            }

            // For all invalid instructions
            else if (result == ArgumentResult::InvalidInstruction) {
                std::cout << "ArgumentResult::InvalidInstruction, please provide a proper instruction.";
            }

            return result;
        }

        // Assign next instruction
        previousInstruction = argv[currentIndex];
    }

    return ArgumentResult::Ok;
}