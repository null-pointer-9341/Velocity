#include "cli.hpp"
#include "cfgen.hpp"

#include <iostream>

// Handles a single argument
ArgumentResult HandleSingleArgument(std::string CurrentArgument, std::string PreviousArgument) {
    // If the currentArgument doesn't exist then return.
    if (CurrentArgument == "") {
        return ArgumentResult::Ok;
    }

    // Check all standalone stuff (like help, version etc.)
    if (CurrentArgument == "-h" || CurrentArgument == "-help") {
        std::cout 
            << "Project version: " << GetVersion() << "\n"
            << "From development branch: " << GetBranch() << "\n";

        return ArgumentResult::Ok;
    }
    else if (CurrentArgument == "-v" || CurrentArgument == "-version" || CurrentArgument == "-ver") {
        std::cout 
            << "";

        return ArgumentResult::Ok;
    }
    else if (CurrentArgument == "-u" || CurrentArgument == "-usage") {
        std::cout 
            << "";

        return ArgumentResult::Ok;
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