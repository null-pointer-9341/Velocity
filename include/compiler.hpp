// A big part of this compiler was taken from my other project Seworks.

#pragma once
#include <string>

// Sets the scene file that gets compiled (-compile)
void SetCompileInput(std::string Path);

// Sets where the .vrtx file gets written (-output)
void SetCompileOutput(std::string Path);

// Compiles the scene if an input was given. Returns false if something went wrong.
bool RunCompile();