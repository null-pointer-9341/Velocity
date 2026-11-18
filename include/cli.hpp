#pragma once
#include <string>

enum class ArgumentResult {
    Ok,                 // Passed
    InvalidArgument,    // The argument was invalid
};

// Handles a single argument
ArgumentResult HandleSingleArgument(std::string CurrentArgument, std::string PreviousArgument);

// This is just to show all commands and their usage.
void PrintCommandUsage();

// Opens the console (primary purpose of existance is to look cool and get rid of needing to write `velocity` per every command)
int RunConsole();

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]);