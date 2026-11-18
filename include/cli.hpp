#pragma once
#include <string>

enum class ArgumentResult {
    Ok, // Passed
    InvalidArgument, // The argument was invalid (e.g. 'Branch Name')
    InvalidInstruction, // The instruction was invalid (e.g. '--naem')
};

// Handles a single argument
ArgumentResult HandleSingleArgument(std::string CurrentArgument, std::string PreviousArgument);

// The main function handles all arguments
ArgumentResult HandleArguments(int argc, char* argv[]);