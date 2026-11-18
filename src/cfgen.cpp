#include "cfgen.hpp"

// These are passed in by meson.build through cpp_args and the fallbacks will only be used if someone compiles without meson (i hope this never happens).
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "unknown"
#endif

#ifndef BRANCH
#define BRANCH "unknown"
#endif

// Gets the project version
std::string GetVersion() {
    return PROJECT_VERSION;
}

// Gets project branch
std::string GetBranch() {
    return BRANCH;
}