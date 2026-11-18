#pragma once
#include <string>

enum class ArgumentResult {
    Ok,                 // Passed
    InvalidArgument,    // The argument was invalid
    CompileFailed,      // The scene couldn't be compiled
    CommandFailed,      // A command (init, config, build or literally anything else) failed
};

// A flag requires a name and a function
struct ValueFlag {
    std::string Name;
    void (*Handler)(std::string Value);
};

// Finds a flag from a table using the argument.
ValueFlag* FindValueFlag(std::string Argument);

// Checks if the flag needs a value after it
bool FlagNeedsValue(std::string Argument);

// Handles a single argument
ArgumentResult HandleSingleArgument(std::string CurrentArgument);

// This is just to show all commands and their usage.
void PrintCommandUsage();

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]);