# Safe Shred

**Multi pass overwriting file shredder to protect sensitive data from recovery.**

## Features

- **Multi pass overwriting** - 3 passes: zeros, ones, and cryptographic random data
- **Metadata destruction** - Randomizes filenames, clears timestamps, corrupts MFT entries
- **SSD optimization** - TRIM command support for modern storage devices
- **Free space wiping** - Removes traces from unallocated disk sectors
- **Batch processing** - Wildcard support (*.txt, file?.doc)
- **System protection** - Prevents accidental deletion of critical OS files
- **Multiple interfaces** - CLI for automation, GUI with drag and drop, context menu for convenience
- **Shell integration** - Right click context menu

## Installation

### Quick Setup

1. **Download latest [release](https://github.com/rmguney/safe-shred/releases)** or build from source.
    - `sash.exe` - Command-line interface
    - `sash_gui.exe` - Graphical interface  
    - `install.bat` / `uninstall.bat` - Context menu setup scripts
2. Extract ZIP to any folder
3. Add folder to Windows Defender exclusions
4. Right-click `install.bat` → "Run as administrator"
5. Right-click any file → "Safe Shred"

### Portable Mode

Run `sash.exe` or `sash_gui.exe` directly without context menu installation.

## Usage

### Command Line

```bash
sash.exe file.txt                    # Basic shredding
sash.exe *.log file1.txt file2.bin   # Multiple files/wildcards  
sash.exe document.txt -p 3           # Custom pass count
sash.exe @filelist.txt               # Read file list
sash.exe --wipe-free C:\             # Wipe drive free space
```

**Options:** `-p N` (passes), `--force` (system files), `--wipe-free` (drive), `@file` (file list)

### GUI Interface

1. Launch `sash_gui.exe`
2. Drag files or click "Add..."
3. Set pass count (1-35, default: 3)
4. Click "Shred" or "Wipe Free Space"

### Context Menu

After installation, right click any file → "Safe Shred" or "Wipe Free Space"

## Security Details

### Overwrite Process

1. **Pass 1:** Fill with zeros (0x00)
2. **Pass 2:** Fill with ones (0xFF)  
3. **Pass 3:** Cryptographically secure random data

### Protection Methods

- **Metadata destruction** - Randomizes filenames, clears timestamps
- **Write-through mode** - Ensures data reaches physical storage
- **MFT corruption** - On SSDs with TRIM support
- **Free space wiping** - Overwrites unallocated sectors

### Limitations

- **SSDs:** Wear leveling may retain data in spare blocks
- **System files:** NTFS journaling, hibernation, swap files may contain traces
- **Backups:** Cloud sync services may have copies

*For maximum security use full disk encryption.*
