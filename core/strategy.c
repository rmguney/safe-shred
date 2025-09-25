#include "strategy.h"
#include <windows.h>
#include <string.h>

static void three_pass_fill(int pass, uint8_t* buf, size_t len) {
    static HCRYPTPROV hProv = 0;
    
    switch (pass) {
        case 0: memset(buf, 0x00, len); break;
        case 1: memset(buf, 0xFF, len); break;
        case 2: 
            if (!hProv) {
                CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
            }
            CryptGenRandom(hProv, (DWORD)len, buf); 
            break;
    }
}

static void gutmann_fill(int pass, uint8_t* buf, size_t len) {
    static HCRYPTPROV hProv = 0;
    
    // Simplified Gutmann
    static const uint8_t patterns[] = {0x00, 0xFF, 0x55, 0xAA, 0x92, 0x49, 0x24};
    if (pass < sizeof(patterns)) {
        memset(buf, patterns[pass], len);
    } else {
        if (!hProv) {
            CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
        }
        CryptGenRandom(hProv, (DWORD)len, buf);
    }
}

const ShredStrategy three_pass_strategy = {
    .name = "three-pass",
    .passes = 3,
    .fill_pass = three_pass_fill
};

const ShredStrategy gutmann_strategy = {
    .name = "gutmann",
    .passes = 35,
    .fill_pass = gutmann_fill
};

// Simple reg
static const ShredStrategy* strategies[] = {
    &three_pass_strategy,
    &gutmann_strategy,
    NULL
};

void register_strategy(const ShredStrategy* strategy) {
    (void)strategy;
}

const ShredStrategy* get_strategy(const char* name) {
    for (int i = 0; strategies[i]; ++i) {
        if (strcmp(strategies[i]->name, name) == 0) {
            return strategies[i];
        }
    }
    return &three_pass_strategy;  // default
}
