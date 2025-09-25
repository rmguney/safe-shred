#ifndef SASH_UTIL_H
#define SASH_UTIL_H

#include <windows.h>

#define SASH_API

// Logging
SASH_API void log_init(void);
SASH_API void log_error(const wchar_t* fmt, ...);
SASH_API void log_info(const wchar_t* fmt, ...);

// Path safety
SASH_API BOOL is_safe_to_delete(const wchar_t* path, int force);

// Privileges
SASH_API BOOL enable_privilege(const wchar_t* privilege);

#endif // SASH_UTIL_H
