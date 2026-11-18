#pragma once
#include <string>

// Git branch currently in.
std::string GetBranch();

// Gets the commit the project was made from.
std::string GetCommit();