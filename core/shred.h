#ifndef SASH_SHRED_H
#define SASH_SHRED_H

#include <windows.h>
#include <stdint.h>
#include <winioctl.h>
#include <wincrypt.h>

#define SASH_API

#ifndef FSCTL_SET_COMPRESSION
#define FSCTL_SET_COMPRESSION   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#endif

#ifndef COMPRESSION_FORMAT_NONE
#define COMPRESSION_FORMAT_NONE 0
#endif

#define ERR_SUCCESS         0
#define ERR_OPEN           -1
#define ERR_WRITE          -2
#define ERR_RENAME         -3
#define ERR_DELETE         -4
#define ERR_PRIVILEGE      -5
#define ERR_SYSTEM_FILE    -6
#define ERR_INVALID_PARAM  -7

// Shred parameters
typedef struct {
    const wchar_t** paths;
    size_t path_count;
    int passes;
    int force_system;
    void (*progress_callback)(const wchar_t* file, int percent, void* ctx);
    void* progress_ctx;
} ShredParams;

// Main API
SASH_API int sash_core_shred(const ShredParams* params);
SASH_API int shred_file(const wchar_t* path, int passes, void (*progress)(int percent));
SASH_API int wipe_free_space(const wchar_t* drive_letter, void (*progress)(int percent));

#endif // SASH_SHRED_H
