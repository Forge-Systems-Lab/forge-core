#include "forge.h"
#include <string.h>

int forge_fast_itoa(size_t val, char *buf) {
    char temp[20]; int i = 0;
    if (val == 0) { buf[0] = '0'; return 1; }
    while (val > 0) { temp[i++] = (val % 10) + '0'; val /= 10; }
    for (int j = 0; j < i; j++) { buf[j] = temp[i - j - 1]; }
    return i;
}

void serialize_row(char *buf, size_t *ptr, size_t row_id, ForgeArena *arena, ForgeConfig *cfg) {
    char num_buf[20]; int n_len;
    memcpy(buf + *ptr, "  {\"r\":", 7); *ptr += 7;
    n_len = forge_fast_itoa(row_id, num_buf);
    memcpy(buf + *ptr, num_buf, n_len); *ptr += n_len;
    memcpy(buf + *ptr, ",\"d\":{", 6); *ptr += 6;

    for (int c = 0; c < cfg->expected_cols; c++) {
        memcpy(buf + *ptr, "\"c", 2); *ptr += 2;
        n_len = forge_fast_itoa(c, num_buf);
        memcpy(buf + *ptr, num_buf, n_len); *ptr += n_len;
        memcpy(buf + *ptr, "\":\"", 3); *ptr += 3;

        ForgeToken *t = &arena->tokens[c];
        memcpy(buf + *ptr, t->data, t->length); *ptr += t->length;
        
        if (c == cfg->expected_cols - 1) {
            memcpy(buf + *ptr, "\"}}", 3); *ptr += 3;
        } else {
            memcpy(buf + *ptr, "\",", 2); *ptr += 2;
        }
    }
}
