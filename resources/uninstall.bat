@echo off
setlocal enabledelayedexpansion
echo Uninstalling Safe Shred...

rem Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running as administrator - proceeding with uninstallation
) else (
    echo ERROR: This script must be run as administrator!
    echo Right-click on uninstall.bat and select "Run as administrator"
    pause
    exit /b 1
)

rem Remove shell extension
echo Removing context menu integration...
echo Removing registry entries for Safe Shred context menu...

rem Remove context menu entries
reg delete "HKEY_CLASSES_ROOT\*\shell\SafeShred" /f 2>nul
reg delete "HKEY_CLASSES_ROOT\Directory\Background\shell\SafeShredFreeSpace" /f 2>nul

if %errorlevel% equ 0 (
    echo Context menu integration removed successfully
) else (
    echo Context menu entries removed (some may not have existed)
)

rem Remove from PATH
echo Removing SafeShred from system PATH...
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH 2^>nul') do set "currentPath=%%b"

if defined currentPath (
    rem Check if SafeShred is in PATH
    echo %currentPath% | find /i "C:\Program Files\SafeShred" >nul
    if %errorlevel% equ 0 (
        echo Removing SafeShred from PATH...
        
        rem Remove all variations of the SafeShred path
        set "newPath=%currentPath%"
        set "newPath=!newPath:;C:\Program Files\SafeShred;=;!"
        set "newPath=!newPath:;C:\Program Files\SafeShred=!"
        set "newPath=!newPath:C:\Program Files\SafeShred;=!"
        set "newPath=!newPath:C:\Program Files\SafeShred=!"
        
        rem Clean up any double semicolons
        :cleanLoop
        set "tempPath=!newPath!"
        set "newPath=!newPath:;;=;!"
        if not "!tempPath!"=="!newPath!" goto cleanLoop
        
        rem Remove leading/trailing semicolons
        if "!newPath:~0,1!"==";" set "newPath=!newPath:~1!"
        if "!newPath:~-1!"==";" set "newPath=!newPath:~0,-1!"
        
        setx /M PATH "!newPath!"
        echo SafeShred removed from PATH
    ) else (
        echo SafeShred was not found in PATH
    )
) else (
    echo Could not read current PATH from registry
)

rem Remove installation directory
echo Removing installation files...
if exist "C:\Program Files\SafeShred" (
    rmdir /S /Q "C:\Program Files\SafeShred"
    echo Installation directory removed
)

echo.
echo Uninstallation complete!
echo Safe Shred has been removed from your system.
echo.
pause
