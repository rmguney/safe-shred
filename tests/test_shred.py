import subprocess
import os
import tempfile
import pytest
import random
import string
from pathlib import Path

# Path to the CLI executable
SASH_CLI = Path(__file__).parent.parent / "build" / "Release" / "sash.exe"

@pytest.fixture
def temp_files():
    """Create temporary test files"""
    with tempfile.TemporaryDirectory() as tmpdir:
        files = []
        # Create files of different sizes
        for size in [1024, 4*1024*1024, 100*1024*1024]:  # 1KB, 4MB, 100MB
            filepath = Path(tmpdir) / f'test_{size}.bin'
            with open(filepath, 'wb') as f:
                # Write random data
                remaining = size
                while remaining > 0:
                    chunk = min(remaining, 1024*1024)
                    f.write(os.urandom(chunk))
                    remaining -= chunk
            files.append(str(filepath))
        yield files

def test_basic_shred(temp_files):
    """Test basic file shredding"""
    for filepath in temp_files:
        assert os.path.exists(filepath)
        
        # Shred the file using CLI
        result = subprocess.run([str(SASH_CLI), filepath], 
                              capture_output=True, text=True)
        assert result.returncode == 0, f"CLI failed: {result.stderr}"
        
        # Verify file no longer exists
        assert not os.path.exists(filepath)

def test_wildcard_shred(temp_files):
    """Test wildcard file shredding"""
    tmpdir = os.path.dirname(temp_files[0])
    pattern = os.path.join(tmpdir, "*.bin")
    
    # Shred all files matching pattern
    result = subprocess.run([str(SASH_CLI), pattern], 
                          capture_output=True, text=True)
    assert result.returncode == 0, f"CLI failed: {result.stderr}"
    
    # Verify all files are gone
    for filepath in temp_files:
        assert not os.path.exists(filepath)

def test_invalid_file():
    """Test shredding non-existent file"""
    result = subprocess.run([str(SASH_CLI), "C:\\nonexistent\\file.txt"], 
                          capture_output=True, text=True)
    assert result.returncode != 0  # Should fail

@pytest.mark.parametrize("passes", [1, 3, 7, 35])
def test_different_passes(temp_files, passes):
    """Test shredding with different pass counts"""
    filepath = temp_files[0]
    
    result = subprocess.run([str(SASH_CLI), "-p", str(passes), filepath], 
                          capture_output=True, text=True)
    assert result.returncode == 0, f"CLI failed: {result.stderr}"
    assert not os.path.exists(filepath)

def test_system_file_protection():
    """Test that system files are protected"""
    system_file = "C:\\Windows\\System32\\kernel32.dll"
    
    result = subprocess.run([str(SASH_CLI), system_file], 
                          capture_output=True, text=True)
    assert result.returncode != 0  # Should fail
    assert os.path.exists(system_file)  # File should still exist

def test_help_output():
    """Test that help is displayed without arguments"""
    result = subprocess.run([str(SASH_CLI)], 
                          capture_output=True, text=True)
    assert result.returncode != 0
    assert "Usage:" in result.stdout

def test_multiple_files(temp_files):
    """Test shredding multiple files at once"""
    # Shred all files at once
    result = subprocess.run([str(SASH_CLI)] + temp_files, 
                          capture_output=True, text=True)
    assert result.returncode == 0, f"CLI failed: {result.stderr}"
    
    # Verify all files are gone
    for filepath in temp_files:
        assert not os.path.exists(filepath)

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
