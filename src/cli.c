#include "forge.h"
#include <getopt.h>

void parse_arguments(int argc, char *argv[], ForgeOps *ops) {
    ops->threads = 8;
    ops->benchmark_only = 0;
    ops->input_path = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "i:t:b")) != -1) {
        switch (opt) {
            case 'i': ops->input_path = optarg; break;
            case 't': ops->threads = atoi(optarg); break;
            case 'b': ops->benchmark_only = 1; break;
        }
    }
}
