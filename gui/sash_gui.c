#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "../core/shred.h"
#include "../core/util.h"

#define IDD_MAIN       100
#define IDC_LISTBOX    1001
#define IDC_ADD        1002
#define IDC_REMOVE     1003
#define IDC_SHRED      1004
#define IDC_PROGRESS   1005
#define IDC_PASSES     1006
#define IDC_WIPE_FREE  1007

static HWND hListBox, hProgress, hPasses;
static HANDLE hShredThread;

typedef struct {
    HWND hwnd;
    ShredParams params;
} ThreadData;

static void UpdateProgress(HWND hwnd, int percent) {
    (void)hwnd;
    SendMessage(hProgress, PBM_SETPOS, percent, 0);
}

static DWORD WINAPI ShredThread(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    int result = sash_core_shred(&data->params);
    
    MessageBoxW(data->hwnd, result == 0 ? L"All files shredded successfully" : 
                L"Some files failed to shred (check log)", L"Safe Shred", 
                result == 0 ? MB_OK | MB_ICONINFORMATION : MB_OK | MB_ICONWARNING);
    
    EnableWindow(GetDlgItem(data->hwnd, IDC_SHRED), TRUE);
    return result;
}

static void freespace_gui_progress(int percent) {
    SendMessage(hProgress, PBM_SETPOS, percent, 0);
}

static DWORD WINAPI FreeSpaceWipeThread(LPVOID param) {
    HWND hwnd = (HWND)param;
    
    int result = wipe_free_space(L"C:\\", freespace_gui_progress);
    
    MessageBoxW(hwnd, result == 0 ? L"Free space wiped successfully" : 
               L"Free space wipe failed (check log)", L"Safe Shred", 
               result == 0 ? MB_OK | MB_ICONINFORMATION : MB_OK | MB_ICONERROR);
    
    EnableWindow(GetDlgItem(hwnd, IDC_WIPE_FREE), TRUE);
    SetWindowTextW(GetDlgItem(hwnd, IDC_WIPE_FREE), L"Wipe &Free Space");
    return result;
}

static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (msg) {
        case WM_INITDIALOG: {
            hListBox = GetDlgItem(hwnd, IDC_LISTBOX);
            hProgress = GetDlgItem(hwnd, IDC_PROGRESS);
            hPasses = GetDlgItem(hwnd, IDC_PASSES);
            
            // icon
            HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
            if (hIcon) {
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            }
            
            SetWindowTextW(hPasses, L"3");
            DragAcceptFiles(hwnd, TRUE);
            return TRUE;
        }
        
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            
            for (UINT i = 0; i < count; ++i) {
                wchar_t path[MAX_PATH];
                DragQueryFileW(hDrop, i, path, MAX_PATH);
                SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)path);
            }
            DragFinish(hDrop);
            return TRUE;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_ADD: {
                    OPENFILENAMEW ofn = {0};
                    wchar_t files[4096] = {0};
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = files;
                    ofn.nMaxFile = 4096;
                    ofn.lpstrFilter = L"All Files\0*.*\0";
                    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
                    
                    if (GetOpenFileNameW(&ofn)) {
                        wchar_t* p = files;
                        wchar_t dir[MAX_PATH];
                        wcscpy_s(dir, MAX_PATH, p);
                        p += wcslen(p) + 1;
                        
                        if (*p) {
                            while (*p) {
                                wchar_t path[MAX_PATH];
                                swprintf_s(path, MAX_PATH, L"%s\\%s", dir, p);
                                SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)path);
                                p += wcslen(p) + 1;
                            }
                        } else {
                            SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)dir);
                        }
                    }
                    return TRUE;
                }
                
                case IDC_REMOVE: {
                    int sel = (int)SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
                    if (sel != LB_ERR) {
                        SendMessageW(hListBox, LB_DELETESTRING, sel, 0);
                    }
                    return TRUE;
                }
                
                case IDC_SHRED: {
                    int count = (int)SendMessageW(hListBox, LB_GETCOUNT, 0, 0);
                    if (count == 0) {
                        MessageBoxW(hwnd, L"No files selected", L"Error", MB_OK | MB_ICONWARNING);
                        return TRUE;
                    }
                    
                    static ThreadData data;
                    data.hwnd = hwnd;
                    data.params.passes = GetDlgItemInt(hwnd, IDC_PASSES, NULL, FALSE);
                    data.params.path_count = count;
                    data.params.paths = malloc(count * sizeof(wchar_t*));
                    
                    for (int i = 0; i < count; ++i) {
                        wchar_t* path = malloc(MAX_PATH * sizeof(wchar_t));
                        SendMessageW(hListBox, LB_GETTEXT, i, (LPARAM)path);
                        ((wchar_t**)data.params.paths)[i] = path;
                    }
                    
                    EnableWindow(GetDlgItem(hwnd, IDC_SHRED), FALSE);
                    hShredThread = CreateThread(NULL, 0, ShredThread, &data, 0, NULL);
                    return TRUE;
                }
                
                case IDC_WIPE_FREE: {
                    // Simple drive selection - could be enhanced with a combo box
                    if (MessageBoxW(hwnd, L"This will wipe free space on C: drive. Continue?", 
                                   L"Free Space Wipe", MB_YESNO | MB_ICONWARNING) == IDYES) {
                        
                        EnableWindow(GetDlgItem(hwnd, IDC_WIPE_FREE), FALSE);
                        SetWindowTextW(GetDlgItem(hwnd, IDC_WIPE_FREE), L"Wiping...");
                        
                        // Create thread for free space wiping
                        HANDLE hWipeThread = CreateThread(NULL, 0, FreeSpaceWipeThread, hwnd, 0, NULL);
                        CloseHandle(hWipeThread);
                    }
                    return TRUE;
                }
            }
            break;
        
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    InitCommonControls();
    enable_privilege(L"SeManageVolumePrivilege");
    
    return (int)DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);
}
