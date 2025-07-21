@echo off
echo Installing Safe Shred...

rem Get the directory where this batch file is located
set "SCRIPT_DIR=%~dp0"
echo Script directory: %SCRIPT_DIR%

rem Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running as administrator - proceeding with installation
) else (
    echo ERROR: This script must be run as administrator!
    echo Right-click on install.bat and select "Run as administrator"
    pause
    exit /b 1
)

rem Create installation directory
if not exist "C:\Program Files\SafeShred" (
    echo Creating installation directory...
    mkdir "C:\Program Files\SafeShred"
)

rem Copy executables from script directory
echo Copying executables...
if exist "%SCRIPT_DIR%sash.exe" (
    copy /Y "%SCRIPT_DIR%sash.exe" "C:\Program Files\SafeShred\"
    echo Copied sash.exe
) else (
    echo ERROR: sash.exe not found in script directory: %SCRIPT_DIR%
    pause
    exit /b 1
)

rem Using embedded icon from sash.exe

rem Install shell extension
echo Installing context menu integration...
echo Adding registry entries for Safe Shred context menu...

rem Add context menu for files
reg add "HKEY_CLASSES_ROOT\*\shell\SafeShred" /ve /d "Safe Shred" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\*\shell\SafeShred" /v "Icon" /d "\"C:\Program Files\SafeShred\sash.exe\",0" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\*\shell\SafeShred\command" /ve /d "\"C:\Program Files\SafeShred\sash.exe\" \"%%1\"" /f >nul 2>&1

rem Add context menu for directory background (free space wipe)
reg add "HKEY_CLASSES_ROOT\Directory\Background\shell\SafeShredFreeSpace" /ve /d "Wipe Free Space" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\Directory\Background\shell\SafeShredFreeSpace" /v "Icon" /d "\"C:\Program Files\SafeShred\sash.exe\",0" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\Directory\Background\shell\SafeShredFreeSpace\command" /ve /d "\"C:\Program Files\SafeShred\sash.exe\" --wipe-free \"%%V\"" /f >nul 2>&1

echo Context menu entries created

rem Refresh the shell to make context menu appear immediately
echo Refreshing Windows Explorer to show context menu...
echo NOTE: Windows Explorer will restart to show the new context menu.
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe

rem Verify registry entries
echo Verifying context menu installation...
reg query "HKEY_CLASSES_ROOT\*\shell\SafeShred" >nul 2>&1
if %errorlevel% equ 0 (
    echo File context menu: INSTALLED
) else (
    echo File context menu: FAILED
)

reg query "HKEY_CLASSES_ROOT\Directory\Background\shell\SafeShredFreeSpace" >nul 2>&1
if %errorlevel% equ 0 (
    echo Directory context menu: INSTALLED
) else (
    echo Directory context menu: FAILED
)

rem Add to PATH (optional)
echo Adding SafeShred to system PATH...
rem Get current PATH from registry to avoid truncation
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH 2^>nul') do set "currentPath=%%b"
if not defined currentPath set "currentPath="

rem Check if SafeShred is already in PATH
echo %currentPath% | find /i "C:\Program Files\SafeShred" >nul
if %errorlevel% neq 0 (
    echo Adding SafeShred to PATH...
    
    rem Safely append to PATH using PowerShell to avoid truncation
    powershell -Command "$currentPath = [Environment]::GetEnvironmentVariable('PATH', 'Machine'); if ($currentPath -notlike '*C:\Program Files\SafeShred*') { $newPath = $currentPath + ';C:\Program Files\SafeShred'; [Environment]::SetEnvironmentVariable('PATH', $newPath, 'Machine'); Write-Host 'SafeShred added to PATH successfully' } else { Write-Host 'SafeShred already in PATH' }"
    
    if %errorlevel% equ 0 (
        echo SafeShred added to PATH successfully
    ) else (
        echo WARNING: Failed to add SafeShred to PATH
    )
) else (
    echo SafeShred is already in PATH
)

echo.
echo Installation complete!
echo Windows Explorer has been refreshed to show the new context menu.
echo You can now:
echo - Right-click any file and select "Safe Shred"
echo - Right-click in empty folder areas and select "Wipe Free Space"
echo - Use 'sash.exe' from command line anywhere
echo - Launch 'sash_gui.exe' for the graphical interface
echo.
echo NOTE: If context menu doesn't appear immediately, restart Explorer or reboot.
echo.
pause
