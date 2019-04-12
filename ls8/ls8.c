#include <stdio.h>
#include <string.h>
#include "cpu.h"

/**
 * Main
 */
int main(int argc, char **argv)
{
    // make sure we're getting passed a file (and possibly a debugging toggle)
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Invalid program call. Correct usage: ./ls8 file_name.ls8\n");
        return 1;
    }

    // turn debugging on if called correctly
    if (argc == 3 && (strcmp(argv[2], "1") == 0 || strcmp(argv[2], "true") == 0))
    {
        debug = 1;
    }

    struct cpu cpu;

    cpu_init(&cpu);
    cpu_load(&cpu, argv[1]);
    cpu_run(&cpu);

    return 0;
}
