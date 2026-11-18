#include "cfgen.hpp"

// These are passed in by meson.build through cpp_args and the fallbacks will only be used if someone compiles without meson (i hope this never happens).
#ifndef COMMIT
#define COMMIT "unknown"
#endif

#ifndef BRANCH
#define BRANCH "unknown"
#endif

// Gets the commit the project was made from
std::string GetCommit() {
    return COMMIT;
}

// Gets project branch
std::string GetBranch() {
    return BRANCH;
}