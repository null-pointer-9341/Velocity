#include "cli.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    // Handle all the arguments
    ArgumentResult result = HandleArguments(argc, argv);
    if (result == ArgumentResult::Ok) {
        return 0;
    } 

    // Whoops...
    return 1;
}