#include "forge.h"
#include <getopt.h>

void print_usage(char *prog_name) {
    printf("Usage: %s -i <input.csv> [options]\n", prog_name);
    printf("Options:\n");
    printf("  -i <file>    Input CSV file (Required)\n");
    printf("  -o <file>    Output JSON file (Default: audit_data.json)\n");
    printf("  -t <int>     Number of threads (Default: 4)\n");
    printf("  -b           Benchmark mode (No file write)\n");
    printf("  -v           Validate only (Check schema only)\n");
    printf("  -h           Show this help message\n");
}

void parse_arguments(int argc, char *argv[], ForgeOps *ops) {
    int opt;
    
    // Initialize defaults
    ops->input_path = NULL;
    ops->output_path = "audit_data.json";
    ops->threads = DEFAULT_THREADS;
    ops->benchmark_only = 0;
    ops->validate_only = 0;

    while ((opt = getopt(argc, argv, "i:o:t:bvh")) != -1) {
        switch (opt) {
            case 'i': ops->input_path = optarg; break;
            case 'o': ops->output_path = optarg; break;
            case 't': ops->threads = atoi(optarg); break;
            case 'b': ops->benchmark_only = 1; break;
            case 'v': ops->validate_only = 1; break;
            case 'h': 
            default:
                print_usage(argv[0]);
                exit(0);
        }
    }

    if (ops->input_path == NULL) {
        fprintf(stderr, "❌ FATAL: Input file (-i) is required.\n");
        print_usage(argv[0]);
        exit(1);
    }
}
