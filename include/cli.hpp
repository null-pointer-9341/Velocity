#pragma once
#include <string>

enum class ArgumentResult {
    Ok,                 // Passed
    InvalidArgument,    // The argument was invalid
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

// Opens the console (primary purpose of existance is to look cool and get rid of needing to write `velocity` per every command)
int RunConsole();

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]);