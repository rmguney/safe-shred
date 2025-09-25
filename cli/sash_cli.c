#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../core/shred.h"
#include "../core/util.h"

static void print_usage(const wchar_t* prog) {
    wprintf(L"Usage: %s [options] file1 [file2 ...]\n", prog);
    wprintf(L"       %s --wipe-free C:\\\n", prog);
    wprintf(L"Options:\n");
    wprintf(L"  -p N        Number of overwrite passes (default: 3)\n");
    wprintf(L"  --force     Allow shredding system files\n");
    wprintf(L"  --wipe-free Wipe free space on drive (e.g., C:\\)\n");
    wprintf(L"  @file       Read file list from file\n");
}

static void progress_callback(const wchar_t* file, int percent, void* ctx) {
    (void)ctx;
    wprintf(L"\r%s: %d%%", file, percent);
    if (percent == 100) wprintf(L"\n");
}

static void freespace_progress_callback(int percent) {
    wprintf(L"\rFree space wipe: %d%%", percent);
    if (percent == 100) wprintf(L"\n");
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    ShredParams params = {0};
    params.passes = 3;
    params.progress_callback = progress_callback;
    
    wchar_t** files = malloc(argc * sizeof(wchar_t*));
    size_t file_count = 0;
    
    for (int i = 1; i < argc; ++i) {
        if (wcscmp(argv[i], L"-p") == 0 && i + 1 < argc) {
            params.passes = _wtoi(argv[++i]);
        } else if (wcscmp(argv[i], L"--force") == 0) {
            params.force_system = 1;
        } else if (wcscmp(argv[i], L"--wipe-free") == 0 && i + 1 < argc) {
            // Free space wiping mode
            wchar_t* drive = argv[++i];
            wprintf(L"Wiping free space on %s...\n", drive);
            
            enable_privilege(L"SeManageVolumePrivilege");
            int result = wipe_free_space(drive, freespace_progress_callback);
            
            free(files);
            return result;
        } else if (argv[i][0] == L'@') {
            // Read file list
            FILE* listfile;
            if (_wfopen_s(&listfile, argv[i] + 1, L"r") == 0) {
                wchar_t line[MAX_PATH];
                while (fgetws(line, MAX_PATH, listfile)) {
                    line[wcscspn(line, L"\r\n")] = 0;
                    if (wcslen(line) > 0) {
                        files[file_count] = _wcsdup(line);
                        file_count++;
                    }
                }
                fclose(listfile);
            }
        } else {
            files[file_count++] = argv[i];
        }
    }
    
    if (file_count == 0) {
        wprintf(L"Error: No files specified\n");
        free(files);
        return 1;
    }
    
    params.paths = (const wchar_t**)files;
    params.path_count = file_count;
    
    // Enable privileges
    enable_privilege(L"SeManageVolumePrivilege");
    
    int result = sash_core_shred(&params);
    
    free(files);
    return result;
}
