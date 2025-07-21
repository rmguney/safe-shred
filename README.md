# Safe Shred (sash)

A secure file deletion utility to protect sensitive data from recovery, for Windows with CLI, GUI, and context menu interfaces.

## Features

### Core Security

- **Secure multi-pass overwriting** (default: 3 passes with 0x00, 0xFF, and cryptographically secure random data)
- **Metadata wiping** (filename randomization, timestamp clearing, MFT entry corruption)
- **Write-through mode** ensures data reaches physical disk storage
- **No Recycle Bin** - files are permanently destroyed

### Advanced Capabilities

- **SSD Support** with TRIM command integration for modern storage
- **Free space wiping** removes traces from unallocated disk sectors
- **System file protection** prevents accidental deletion of critical OS files
- **Batch processing** with wildcard pattern support (*.txt, file?.doc)
- **Shell integration** with right-click context menu

### Utilities

- **Dual interfaces** - command-line for automation, GUI for ease of use
- **Real-time progress** reporting with detailed logging
- **Drag-and-drop** support in GUI interface
- **Comprehensive error handling** and validation

## Quick Start

### Download

Download the latest release from GitHub releases or build from source.

**Complete Package Includes:**

- `sash.exe` - Command-line interface (~185 KB)
- `sash_gui.exe` - Graphical interface (~183 KB)
- `sash.ico` - Application icon
- `install.bat` - Automated installer script
- `uninstall.bat` - Removal script

### Installation

1. **Download & Extract**: Extract the complete ZIP package to a folder
2. **Add Antivirus Exclusion**: Add the folder to Windows Defender exclusions (Settings → Update & Security → Windows Security → Virus & threat protection → Exclusions)
3. **Install**: Right-click `install.bat` and select "Run as administrator"
4. **Use**: Right-click any file → "Safe Shred" or launch `sash_gui.exe`

### Portable Use

For portable operation without installation:

- Extract ZIP to any folder
- Run `sash.exe` or `sash_gui.exe` directly
- No registry changes or system integration

## Building from Source

### Requirements

- Visual Studio 2022 (for manual build)
- CMake 3.24+ (for manual build)
- Python 3.8+ (for testing)

### Build Process

```bash
git clone <repository-url>
cd safe-shred
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

**Build outputs:**

- `build/Release/sash.exe` - CLI executable
- `build/Release/sash_gui.exe` - GUI executable
- `build/Release/sash_core_static.lib` - Core library

## Usage

### CLI

```bash
# Basic file shredding
sash.exe document.txt

# Multiple files and wildcards
sash.exe file1.txt file2.bin *.log

# Custom pass count (1-35 passes supported)
sash.exe sensitive.dat -p 7

# Read file list from text file
sash.exe @filelist.txt

# Force shredding system files (just dont)
sash.exe system.dll --force

# Wipe free space on entire drive
sash.exe --wipe-free C:\
```

**CLI Options:**

- `-p N` - Number of overwrite passes (default: 3)
- `--force` - Allow shredding system files
- `--wipe-free` - Wipe free space on drive
- `@file` - Read file list from file

### GUI

1. Launch `sash_gui.exe`
2. **Add files**: Drag-and-drop or use "Add..." button
3. **Configure**: Set number of passes (1-35, default: 3)
4. **Execute**: Click "Shred" to delete files or "Wipe Free Space" for drive cleaning
5. **Monitor**: Watch real-time progress and detailed logging

## Shell Integration

### Automated Installation

The `install.bat` script provides:

- Copies executables to `C:\Program Files\SafeShred\`
- Adds context menu integration via registry
- Adds SafeShred to system PATH
- Includes application icon in context menus

### Context Menu Usage

After installation:

- **Right-click any file** → "Safe Shred"
- **Right-click in empty folder** → "Wipe Free Space"

### Uninstall

Run `uninstall.bat` as administrator to completely remove Safe Shred from your system.

## Development & Testing

### Running Tests

```bash
# Install Python dependencies
pip install pytest pytest-cov

# Run all tests
cd tests
python -m pytest test_shred.py -v

# Run with coverage
python -m pytest test_shred.py --cov-report=html
```

### Test Coverage

The test suite includes:

- Basic file shredding (1KB, 4MB, 100MB files)
- Wildcard pattern processing
- Multiple file handling
- Custom pass count validation (1, 3, 7, 35 passes)
- System file protection
- Error handling for invalid files
- CLI help output verification

### Continuous Integration

GitHub Actions workflow automatically:

- Builds on Windows with Visual Studio 2022
- Runs complete test suite
- Validates executable sizes and functionality
- Creates release packages
- Performs security scanning

## Security Implementation

### Overwrite Process

1. **Pass 1**: Fill with 0x00 (zeros)
2. **Pass 2**: Fill with 0xFF (ones)  
3. **Pass 3**: Fill with cryptographically secure random data (CryptGenRandom)

### Metadata Destruction

- **Filename randomization**: Multiple rename cycles with random names
- **Timestamp clearing**: Creation, modification, and access times zeroed
- **MFT corruption**: On SSDs with TRIM support, corrupts Master File Table entries

### Advanced Protection

- **Write-through mode**: FILE_FLAG_WRITE_THROUGH ensures data reaches physical storage
- **Compression disabled**: FILE_FLAG_NO_BUFFERING prevents OS caching
- **Free space wiping**: Overwrites unallocated disk sectors to remove file traces
- **TRIM support**: Notifies SSD controllers to mark blocks as unused
- **System protection**: Prevents deletion of critical Windows files

### Verification

- File existence checked after each operation
- Write operations verified for completion
- Error handling for locked or protected files
- Comprehensive logging of all operations

## Security Limitations

**Important Security Considerations:**

- **SSDs**: Modern SSDs with wear leveling may retain data in spare blocks
- **Journaling**: NTFS journaling may keep temporary copies
- **Hibernation/Paging**: System swap files may contain traces
- **RAM**: Data may persist in memory after deletion
- **Backups**: Cloud sync and backup services may have copies

**For maximum security**: Use full disk encryption and secure disk wiping tools for entire drives.

## License

This project is released under the MIT License. See LICENSE file for details.
