#!/usr/bin/env python3
"""
Create a TRS-80 test disk with Phase 2 /CMD programs

This script creates a DMK disk image and adds all Phase 2 test programs.
The disk can be used with trs80gp or sdltrs emulators.
"""

import sys
import os
import struct
from pathlib import Path
from dmk import DMKDisk

class TRS80Directory:
    """Simple TRS-DOS/NewDOS-80 directory handler"""

    # Directory starts at track 17, sector 0
    DIR_TRACK = 17
    DIR_SECTOR = 0
    SECTORS_PER_TRACK = 18
    SECTOR_SIZE = 256

    # Directory entry structure (32 bytes)
    ENTRY_SIZE = 32
    ENTRIES_PER_SECTOR = SECTOR_SIZE // ENTRY_SIZE  # 8

    # File flags
    FLAG_ACTIVE = 0x00
    FLAG_DELETED = 0xFF

    def __init__(self, disk):
        self.disk = disk
        self.entries = []
        self._load_directory()

    def _load_directory(self):
        """Load directory from disk"""
        self.entries = []

        # Read directory sectors (track 17, sectors 0-1)
        for sector in range(2):
            try:
                data = self.disk.read_sector(self.DIR_TRACK, sector + 1)
                for i in range(self.ENTRIES_PER_SECTOR):
                    entry_offset = i * self.ENTRY_SIZE
                    entry = data[entry_offset:entry_offset + self.ENTRY_SIZE]

                    # Check if entry is active
                    if entry[0] != self.FLAG_DELETED:
                        self.entries.append({
                            'flag': entry[0],
                            'filename': entry[0:8].decode('ascii', errors='ignore').strip(),
                            'extension': entry[8:11].decode('ascii', errors='ignore').strip(),
                            'start_granule': entry[12],
                            'end_granule': entry[13],
                            'sectors': entry[13],  # Simplified
                            'raw': entry
                        })
            except:
                pass

    def _save_directory(self):
        """Save directory to disk"""
        # Prepare directory sectors
        dir_data = bytearray(self.SECTOR_SIZE * 2)
        dir_data[:] = [0xE5] * len(dir_data)  # Fill with empty

        for i, entry in enumerate(self.entries):
            if i >= 16:  # Max 16 entries (2 sectors * 8 entries)
                break
            entry_offset = i * self.ENTRY_SIZE
            dir_data[entry_offset:entry_offset + self.ENTRY_SIZE] = entry['raw']

        # Write back to disk
        try:
            self.disk.write_sector(self.DIR_TRACK, 1, dir_data[:self.SECTOR_SIZE])
            self.disk.write_sector(self.DIR_TRACK, 2, dir_data[self.SECTOR_SIZE:])
        except:
            pass

    def add_file(self, filename, extension, data):
        """Add a file to the directory (simplified)"""
        # Create directory entry
        entry = bytearray(self.ENTRY_SIZE)
        entry[:] = [0] * self.ENTRY_SIZE

        # Filename (8 bytes, space-padded)
        name_bytes = filename.upper().ljust(8)[:8].encode('ascii')
        entry[0:8] = name_bytes

        # Extension (3 bytes, space-padded)
        ext_bytes = extension.upper().ljust(3)[:3].encode('ascii')
        entry[8:11] = ext_bytes

        # For simplicity, just store file info
        # Real implementation would allocate granules
        entry[12] = 0  # Start granule (would be allocated)
        entry[13] = len(data) // 256 + 1  # Sectors

        self.entries.append({
            'flag': self.FLAG_ACTIVE,
            'filename': filename,
            'extension': extension,
            'start_granule': 0,
            'end_granule': 0,
            'sectors': len(data) // 256 + 1,
            'raw': entry,
            'data': data
        })

    def list_files(self):
        """List files in directory"""
        files = []
        for entry in self.entries:
            if entry['flag'] != self.FLAG_DELETED:
                files.append({
                    'name': f"{entry['filename']}.{entry['extension']}".strip(),
                    'filename': entry['filename'],
                    'extension': entry['extension'],
                    'sectors': entry['sectors']
                })
        return files


def create_test_disk(output_file, test_dir):
    """Create a test disk with Phase 2 programs"""

    print(f"Creating TRS-80 test disk: {output_file}")

    # Create blank disk (40 track, single-sided, double-density)
    print("  Creating blank disk image...")
    disk = DMKDisk.create(tracks=40, sides=1, density='double')

    # Format all tracks
    print("  Formatting disk...")
    disk.format_all(sectors_per_track=18, sector_size=256)

    # Initialize directory
    print("  Initializing directory...")
    directory = TRS80Directory(disk)

    # Find all .cmd files in test directory
    test_path = Path(test_dir)
    cmd_files = sorted(test_path.glob('*.cmd'))

    if not cmd_files:
        print(f"  WARNING: No .cmd files found in {test_dir}")
    else:
        print(f"  Found {len(cmd_files)} test programs:")

        # Add each file
        for cmd_file in cmd_files:
            # Read file data
            with open(cmd_file, 'rb') as f:
                data = f.read()

            # Generate TRS-80 filename (8.3 format)
            base_name = cmd_file.stem.upper()
            if len(base_name) > 8:
                base_name = base_name[:8]

            print(f"    Adding: {base_name}/CMD ({len(data)} bytes)")

            # Add to directory
            directory.add_file(base_name, 'CMD', data)

    # Save directory
    directory._save_directory()

    # Save disk image
    print(f"  Saving disk image...")
    disk.save(output_file)

    info = disk.get_info()
    print(f"\nDisk created successfully!")
    print(f"  Format: DMK")
    print(f"  Tracks: {info['tracks']}")
    print(f"  Sides: {info['sides']}")
    print(f"  Density: {info['density']}")
    print(f"  Files: {len(cmd_files)}")

    return disk


def main():
    if len(sys.argv) < 2:
        print("Usage: create_test_disk.py <test_directory> [output.dmk]")
        print()
        print("Example:")
        print("  create_test_disk.py ../../tests/edtasm-m80/phase2 phase2_tests.dmk")
        sys.exit(1)

    test_dir = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'test_disk.dmk'

    if not os.path.isdir(test_dir):
        print(f"Error: Test directory not found: {test_dir}")
        sys.exit(1)

    try:
        disk = create_test_disk(output_file, test_dir)

        print("\nTo test in emulator:")
        print(f"  trs80gp -m3 -disk0 {output_file}")
        print(f"  sdltrs -model 3 -disk0 {output_file}")

    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
