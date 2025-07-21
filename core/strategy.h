#ifndef SASH_STRATEGY_H
#define SASH_STRATEGY_H

#include <stdint.h>
#include <stddef.h>
#include <windows.h>
#include <wincrypt.h>

typedef struct {
    const char* name;
    int passes;
    void (*fill_pass)(int pass, uint8_t* buf, size_t len);
} ShredStrategy;

// Built-in strategies
extern const ShredStrategy three_pass_strategy;
extern const ShredStrategy gutmann_strategy;

// Strategy registry
void register_strategy(const ShredStrategy* strategy);
const ShredStrategy* get_strategy(const char* name);

#endif // SASH_STRATEGY_H
