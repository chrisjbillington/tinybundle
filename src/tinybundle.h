#include <string.h>
#include <errno.h>
#include <stdio.h>

// A generated file which defines BOOTSTRAPPER_SIZE
#include "bootstrapper.h"

#define PATH_MAX 4096
#define BLOCK_SIZE 512

// System-specific code
#ifdef _WIN32
#define PATHSEP '\\'
#define CMD_MAX = 8191
#else
#define PATHSEP '/'
#endif

