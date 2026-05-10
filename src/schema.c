#include "forge.h" // <--- MUST BE FIRST
#include <string.h>
#include <stdio.h>

int forge_load_schema(const char *path, ForgeConfig *config) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    int col_idx = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"delimiter\"")) {
            char d;
            // Scan for the char inside quotes
            char *quote_ptr = strchr(line, '"');
            if (quote_ptr) {
                char *second_quote = strchr(quote_ptr + 1, '"');
                if (second_quote) {
                    char *third_quote = strchr(second_quote + 1, '"');
                    if (third_quote) config->delimiter = *(third_quote + 1);
                }
            }
            // Fallback scanner if logic above is too complex for simple JSON
            if (strstr(line, "\",\"")) config->delimiter = ',';
        }
        if (strstr(line, "\"column_count\"")) {
            sscanf(line, " \"column_count\": %d", &config->expected_cols);
        }
        if (strstr(line, "\"type\": \"int\"")) config->types[col_idx] = FORGE_INT;
        if (strstr(line, "\"type\": \"string\"")) config->types[col_idx] = FORGE_STRING;
        if (strstr(line, "\"required\": 1")) {
            config->required[col_idx] = 1;
            col_idx++;
        } else if (strstr(line, "\"required\": 0")) {
            config->required[col_idx] = 0;
            col_idx++;
        }
    }
    fclose(f);
    return 0;
}
