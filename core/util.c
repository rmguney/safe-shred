#include "util.h"
#include <stdio.h>
#include <stdarg.h>
#include <shlobj.h>

static FILE* log_file = NULL;

void log_init(void) {
    wchar_t logpath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, logpath))) {
        wcscat_s(logpath, MAX_PATH, L"\\sash");
        CreateDirectoryW(logpath, NULL);
        wcscat_s(logpath, MAX_PATH, L"\\sash.log");
        _wfopen_s(&log_file, logpath, L"a");
    }
}

static void log_write(const wchar_t* level, const wchar_t* fmt, va_list args) {
    if (!log_file) return;
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(log_file, L"[%04d-%02d-%02d %02d:%02d:%02d] %s: ",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, level);
    vfwprintf(log_file, fmt, args);
    fwprintf(log_file, L"\n");
    fflush(log_file);
}

void log_error(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_write(L"ERROR", fmt, args);
    va_end(args);
}

void log_info(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_write(L"INFO", fmt, args);
    va_end(args);
}

BOOL is_safe_to_delete(const wchar_t* path, int force) {
    if (force) return TRUE;
    
    wchar_t system[MAX_PATH], programfiles[MAX_PATH];
    GetSystemDirectoryW(system, MAX_PATH);
    SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, programfiles);
    
    if (_wcsnicmp(path, system, wcslen(system)) == 0 ||
        _wcsnicmp(path, programfiles, wcslen(programfiles)) == 0) {
        return FALSE;
    }
    
    return TRUE;
}

BOOL enable_privilege(const wchar_t* privilege) {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;
    
    LUID luid;
    if (!LookupPrivilegeValueW(NULL, privilege, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }
    
    TOKEN_PRIVILEGES tp = {0};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    BOOL result = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return result && GetLastError() == ERROR_SUCCESS;
}
