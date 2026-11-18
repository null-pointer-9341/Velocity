#pragma once

// Creates the source folders (src/server, src/client, src/shared)
bool RunInit();

// Creates the config folder (config/project.json)
bool RunConfig();

// Compiles all of the scripts in the source folders into a .vrtx file
bool RunBuild(bool includeFramework = false);