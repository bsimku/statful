#ifndef BLOCK_COMMON_H
#define BLOCK_COMMON_H

#define BLOCK_PARAM(param_name, fmt, value)                      \
    if (strcmp(name, param_name) == 0) {                         \
        const int sz = snprintf(output, size, fmt, value);       \
                                                                 \
        if (sz >= size)                                          \
            return false;                                        \
                                                                 \
        output += sz;                                            \
                                                                 \
        return output;                                           \
    }                                                            \

#endif
