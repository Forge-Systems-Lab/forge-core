#include "forge.h"
#include <string.h>

void serialize_row(char *buf, size_t *ptr, const char *line_start, size_t line_len) {
    // Zero-overhead JSON framing
    *ptr += sprintf(buf + *ptr, "{\"data\":\"%.*s\"}\n", (int)line_len, line_start);
}
