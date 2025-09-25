#include "shred.h"
#include "strategy.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Optimized buffer size for maximum performance (4 MiB for modern SSDs)
#define BUF_SIZE (4 * 1024 * 1024)  // 4 MiB for optimal SSD performance

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

// Pre-allocated static buffer for better performance
static uint8_t g_write_buffer[BUF_SIZE];
static HCRYPTPROV g_hProv = 0;

static BOOL fill_buffer(int pass, uint8_t* buf, size_t len, int total_passes) {
    (void)total_passes;
    
    switch (pass % 3) {
        case 0:
            // Use fast memset for 0x00 pattern
            memset(buf, 0x00, len);
            break;
        case 1:
            // Use fast memset for 0xFF pattern
            memset(buf, 0xFF, len);
            break;
        case 2:
            // Initialize crypto provider once for better performance
            if (!g_hProv) {
                if (!CryptAcquireContext(&g_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
                    return FALSE;
                }
            }
            return CryptGenRandom(g_hProv, (DWORD)len, buf);
    }
    return TRUE;
}

static int secure_overwrite(HANDLE hFile, int passes, void (*progress)(int percent)) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) return ERR_WRITE;
    
    // Use pre-allocated static buffer for better performance
    uint8_t* buf = g_write_buffer;
    
    for (int pass = 0; pass < passes; ++pass) {
        LARGE_INTEGER zero = {0};
        SetFilePointerEx(hFile, zero, NULL, FILE_BEGIN);
        
        for (int64_t done = 0; done < size.QuadPart; done += BUF_SIZE) {
            size_t chunk = (size_t)min(BUF_SIZE, size.QuadPart - done);
            
            if (!fill_buffer(pass, buf, chunk, passes)) {
                return ERR_WRITE;
            }
            
            DWORD written;
            if (!WriteFile(hFile, buf, (DWORD)chunk, &written, NULL) || written != chunk) {
                return ERR_WRITE;
            }
            
            // Update progress less frequently for better performance
            if (progress && (done % (BUF_SIZE * 4) == 0)) {
                int pct = (int)((pass * 100 + (done * 100 / size.QuadPart)) / passes);
                progress(pct);
            }
        }
        
        // Force write to disk for each pass
        FlushFileBuffers(hFile);
    }
    
    return ERR_SUCCESS;
}

static int wipe_metadata(const wchar_t* path) {
    // Get full path to ensure we have absolute path
    wchar_t fullpath[MAX_PATH];
    if (!GetFullPathNameW(path, MAX_PATH, fullpath, NULL)) {
        return ERR_RENAME;
    }
    
    // Extract directory
    wchar_t dir[MAX_PATH];
    wcscpy_s(dir, MAX_PATH, fullpath);
    wchar_t* lastslash = wcsrchr(dir, L'\\');
    if (!lastslash) {
        // No directory separator, use current directory
        wcscpy_s(dir, MAX_PATH, L".");
    } else {
        *lastslash = L'\0';
    }
    
    // Initialize random seed
    static int seed_initialized = 0;
    if (!seed_initialized) {
        srand((unsigned int)GetTickCount());
        seed_initialized = 1;
    }
    
    // Rename 3 times with random names
    wchar_t currentpath[MAX_PATH];
    wcscpy_s(currentpath, MAX_PATH, fullpath);
    
    for (int i = 0; i < 3; ++i) {
        wchar_t randname[32];
        swprintf_s(randname, 32, L"~%08X%08X.tmp", rand(), rand());
        
        wchar_t newpath[MAX_PATH];
        swprintf_s(newpath, MAX_PATH, L"%s\\%s", dir, randname);
        
        if (!MoveFileW(currentpath, newpath)) {
            log_error(L"Failed to rename %s to %s", currentpath, newpath);
            return ERR_RENAME;
        }
        wcscpy_s(currentpath, MAX_PATH, newpath);
    }
    
    // Final deletion
    if (!DeleteFileW(currentpath)) {
        log_error(L"Failed to delete %s", currentpath);
        return ERR_DELETE;
    }
    return ERR_SUCCESS;
}

int shred_file(const wchar_t* path, int passes, void (*progress)(int percent)) {
    // Check if system file
    if (!is_safe_to_delete(path, 0)) return ERR_SYSTEM_FILE;
    
    // Open with write-through
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return ERR_OPEN;
    
    // Disable compression
    USHORT compression = COMPRESSION_FORMAT_NONE;
    DWORD bytesReturned;
    DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &compression, sizeof(compression),
                    NULL, 0, &bytesReturned, NULL);
    
    // Enable TRIM support for SSDs
    FILE_ZERO_DATA_INFORMATION zeroData;
    zeroData.FileOffset.QuadPart = 0;
    GetFileSizeEx(hFile, &zeroData.BeyondFinalZero);
    DeviceIoControl(hFile, FSCTL_SET_ZERO_DATA, &zeroData, sizeof(zeroData),
                    NULL, 0, &bytesReturned, NULL);
    
    // Overwrite data
    int result = secure_overwrite(hFile, passes, progress);
    
    // Truncate
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    SetEndOfFile(hFile);
    
    CloseHandle(hFile);
    
    if (result == ERR_SUCCESS) {
        result = wipe_metadata(path);
    }
    
    return result;
}

SASH_API int sash_core_shred(const ShredParams* params) {
    if (!params || !params->paths) return ERR_INVALID_PARAM;
    
    log_init();
    int failed = 0;
    
    for (size_t i = 0; i < params->path_count; ++i) {
        const wchar_t* path = params->paths[i];
        
        // Handle wildcards
        if (wcschr(path, L'*') || wcschr(path, L'?')) {
            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW(path, &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        wchar_t fullpath[MAX_PATH];
                        wchar_t dir[MAX_PATH];
                        wcscpy_s(dir, MAX_PATH, path);
                        wchar_t* lastslash = wcsrchr(dir, L'\\');
                        if (lastslash) {
                            *lastslash = L'\0';
                            swprintf_s(fullpath, MAX_PATH, L"%s\\%s", dir, fd.cFileName);
                        } else {
                            wcscpy_s(fullpath, MAX_PATH, fd.cFileName);
                        }
                        
                        int result = shred_file(fullpath, params->passes ? params->passes : 3, NULL);
                        if (result != ERR_SUCCESS) {
                            log_error(L"Failed to shred %s: %d", fullpath, result);
                            failed++;
                        }
                    }
                } while (FindNextFileW(hFind, &fd));
                FindClose(hFind);
            }
        } else {
            int result = shred_file(path, params->passes ? params->passes : 3, NULL);
            if (result != ERR_SUCCESS) {
                log_error(L"Failed to shred %s: %d", path, result);
                failed++;
            }
        }
    }
    
    return failed > 0 ? 1 : 0;
}

SASH_API int wipe_free_space(const wchar_t* drive_letter, void (*progress)(int percent)) {
    wchar_t temp_file[MAX_PATH];
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    
    // Get available free space
    if (!GetDiskFreeSpaceExW(drive_letter, &free_bytes, &total_bytes, &total_free_bytes)) {
        log_error(L"Failed to get disk space for %s", drive_letter);
        return ERR_OPEN;
    }
    
    // Create temporary file to fill free space
    swprintf_s(temp_file, MAX_PATH, L"%s\\~sash_freespace_%08X.tmp", drive_letter, GetTickCount());
    
    HANDLE hFile = CreateFileW(temp_file, GENERIC_WRITE,
                              0, NULL, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        log_error(L"Failed to create temporary file for free space wiping");
        return ERR_OPEN;
    }
    
    // Fill free space with random data (optimized)
    uint8_t* buf = g_write_buffer;  // Use pre-allocated buffer
    
    // Initialize crypto provider if needed
    if (!g_hProv) {
        CryptAcquireContext(&g_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    }
    
    ULARGE_INTEGER written = {0};
    ULARGE_INTEGER target = free_bytes;
    target.QuadPart = (target.QuadPart * 95) / 100;  // Leave 5% free to avoid system issues
    
    while (written.QuadPart < target.QuadPart) {
        ULARGE_INTEGER remaining;
        remaining.QuadPart = target.QuadPart - written.QuadPart;
        DWORD chunk_size = (DWORD)min(BUF_SIZE, remaining.QuadPart);
        
        // Fill buffer with random data
        CryptGenRandom(g_hProv, chunk_size, buf);
        
        DWORD bytes_written;
        if (!WriteFile(hFile, buf, chunk_size, &bytes_written, NULL)) {
            break;
        }
        
        written.QuadPart += bytes_written;
        
        if (progress && (written.QuadPart % (BUF_SIZE * 8) == 0)) {
            int pct = (int)((written.QuadPart * 100) / target.QuadPart);
            progress(pct);
        }
    }
    
    CloseHandle(hFile);  // This will delete the temp file
    
    log_info(L"Free space wiped on %s: %I64u bytes", drive_letter, written.QuadPart);
    return ERR_SUCCESS;
}
