#!/usr/bin/env python3
"""
DMK (David M. Keil) disk image format handler
Supports TRS-80 Model I/III/4 disk images
"""

import struct
import sys
from typing import List, Tuple, Optional

class DMKDisk:
    """DMK disk image handler"""

    # DMK header constants
    HEADER_SIZE = 16
    WRITE_PROTECTED = 0xFF
    NOT_WRITE_PROTECTED = 0x00

    # Density flags
    SINGLE_DENSITY = 0x00
    DOUBLE_DENSITY = 0x40
    IGNORE_DENSITY = 0x80
    SINGLE_SIDED = 0x00
    DOUBLE_SIDED = 0x10

    def __init__(self):
        self.write_protected = False
        self.tracks = 0
        self.track_length = 0
        self.disk_options = 0
        self.track_data = []

    @classmethod
    def create(cls, tracks=40, sides=1, density='double'):
        """Create a new blank DMK disk image"""
        disk = cls()
        disk.write_protected = False
        disk.tracks = tracks * sides

        # Standard track lengths
        if density == 'single':
            disk.track_length = 0x0CC0  # Single density
            disk.disk_options = cls.SINGLE_DENSITY
        else:  # double
            disk.track_length = 0x1900  # Double density
            disk.disk_options = cls.DOUBLE_DENSITY

        if sides == 2:
            disk.disk_options |= cls.DOUBLE_SIDED

        # Create blank tracks
        for _ in range(disk.tracks):
            track = bytearray(disk.track_length)
            disk.track_data.append(track)

        return disk

    @classmethod
    def load(cls, filename):
        """Load a DMK disk image from file"""
        disk = cls()

        with open(filename, 'rb') as f:
            # Read header
            header = f.read(cls.HEADER_SIZE)
            if len(header) < cls.HEADER_SIZE:
                raise ValueError("Invalid DMK file: header too short")

            # Parse header
            disk.write_protected = header[0] != cls.NOT_WRITE_PROTECTED
            disk.tracks = header[1]
            disk.track_length = struct.unpack('<H', header[2:4])[0]
            disk.disk_options = header[4]

            # Read track data
            for _ in range(disk.tracks):
                track = f.read(disk.track_length)
                if len(track) < disk.track_length:
                    raise ValueError("Invalid DMK file: truncated track data")
                disk.track_data.append(bytearray(track))

        return disk

    def save(self, filename):
        """Save DMK disk image to file"""
        with open(filename, 'wb') as f:
            # Write header
            header = bytearray(self.HEADER_SIZE)
            header[0] = self.WRITE_PROTECTED if self.write_protected else self.NOT_WRITE_PROTECTED
            header[1] = self.tracks
            struct.pack_into('<H', header, 2, self.track_length)
            header[4] = self.disk_options
            # Bytes 5-15 are reserved (zeros)
            f.write(header)

            # Write track data
            for track in self.track_data:
                f.write(track)

    def format_track(self, track_num, sectors=18, sector_size=256):
        """Format a track with standard sector layout"""
        if track_num >= len(self.track_data):
            raise ValueError(f"Track {track_num} out of range")

        track = bytearray(self.track_length)

        # IDAM pointer table at start of track
        # Each entry is 2 bytes: offset to sector IDAM
        ptr_offset = 0
        data_offset = 128  # Start data after pointer table

        for sector in range(sectors):
            # IDAM pointer (little-endian, bit 15 set for double density)
            idam_offset = data_offset
            if self.disk_options & self.DOUBLE_DENSITY:
                idam_offset |= 0x8000
            struct.pack_into('<H', track, ptr_offset, idam_offset)
            ptr_offset += 2

            # Write IDAM (ID Address Mark)
            # Format: FE <track> <side> <sector> <size> <crc1> <crc2>
            track[data_offset] = 0xFE  # IDAM
            track[data_offset + 1] = track_num  # Track number
            track[data_offset + 2] = 0  # Side (0 or 1)
            track[data_offset + 3] = sector + 1  # Sector number (1-based)
            track[data_offset + 4] = 1  # Size code (0=128, 1=256, 2=512, 3=1024)
            track[data_offset + 5] = 0  # CRC (simplified)
            track[data_offset + 6] = 0  # CRC
            data_offset += 7

            # Write Data Address Mark
            track[data_offset] = 0xFB  # DAM
            data_offset += 1

            # Write sector data (filled with 0xE5 = empty)
            for i in range(sector_size):
                track[data_offset] = 0xE5
                data_offset += 1

            # Write CRC
            track[data_offset] = 0
            track[data_offset + 1] = 0
            data_offset += 2

        # Mark end of pointer table
        struct.pack_into('<H', track, ptr_offset, 0x0000)

        self.track_data[track_num] = track

    def format_all(self, sectors_per_track=18, sector_size=256):
        """Format all tracks"""
        for track_num in range(self.tracks):
            self.format_track(track_num, sectors_per_track, sector_size)

    def read_sector(self, track, sector):
        """Read a sector from the disk"""
        if track >= len(self.track_data):
            raise ValueError(f"Track {track} out of range")

        track_data = self.track_data[track]

        # Find sector in IDAM pointer table
        ptr_offset = 0
        while ptr_offset < 128:
            idam_ptr = struct.unpack_from('<H', track_data, ptr_offset)[0]
            if idam_ptr == 0:
                break  # End of table

            # Mask off density bit
            idam_offset = idam_ptr & 0x3FFF

            # Read IDAM
            if track_data[idam_offset] == 0xFE:
                idam_track = track_data[idam_offset + 1]
                idam_sector = track_data[idam_offset + 3]
                sector_size_code = track_data[idam_offset + 4]

                if idam_sector == sector:
                    # Found it! Read data
                    sector_size = 128 << sector_size_code
                    dam_offset = idam_offset + 7
                    if track_data[dam_offset] == 0xFB:
                        data_offset = dam_offset + 1
                        return bytes(track_data[data_offset:data_offset + sector_size])

            ptr_offset += 2

        raise ValueError(f"Sector {sector} not found on track {track}")

    def write_sector(self, track, sector, data):
        """Write a sector to the disk"""
        if track >= len(self.track_data):
            raise ValueError(f"Track {track} out of range")

        track_data = self.track_data[track]

        # Find sector in IDAM pointer table
        ptr_offset = 0
        while ptr_offset < 128:
            idam_ptr = struct.unpack_from('<H', track_data, ptr_offset)[0]
            if idam_ptr == 0:
                break  # End of table

            # Mask off density bit
            idam_offset = idam_ptr & 0x3FFF

            # Read IDAM
            if track_data[idam_offset] == 0xFE:
                idam_sector = track_data[idam_offset + 3]
                sector_size_code = track_data[idam_offset + 4]

                if idam_sector == sector:
                    # Found it! Write data
                    sector_size = 128 << sector_size_code
                    if len(data) > sector_size:
                        raise ValueError(f"Data too large for sector (max {sector_size} bytes)")

                    dam_offset = idam_offset + 7
                    data_offset = dam_offset + 1

                    # Write data
                    for i, byte in enumerate(data):
                        track_data[data_offset + i] = byte

                    # Fill remainder with 0xE5
                    for i in range(len(data), sector_size):
                        track_data[data_offset + i] = 0xE5

                    return

            ptr_offset += 2

        raise ValueError(f"Sector {sector} not found on track {track}")

    def get_info(self):
        """Get disk information"""
        density = "Double" if (self.disk_options & self.DOUBLE_DENSITY) else "Single"
        sides = 2 if (self.disk_options & self.DOUBLE_SIDED) else 1
        physical_tracks = self.tracks // sides

        return {
            'write_protected': self.write_protected,
            'tracks': physical_tracks,
            'sides': sides,
            'density': density,
            'track_length': self.track_length,
            'total_tracks': self.tracks
        }


if __name__ == '__main__':
    # Test: Create a blank disk
    print("Creating test DMK disk...")
    disk = DMKDisk.create(tracks=40, sides=1, density='double')
    disk.format_all(sectors_per_track=18, sector_size=256)
    disk.save('test.dmk')
    print(f"Created test.dmk")
    print(f"Info: {disk.get_info()}")

    # Test: Read it back
    print("\nReading test disk...")
    disk2 = DMKDisk.load('test.dmk')
    print(f"Info: {disk2.get_info()}")

    # Test: Write and read a sector
    print("\nTesting sector I/O...")
    test_data = b"Hello, TRS-80!" + b"\x00" * (256 - 14)
    disk2.write_sector(0, 1, test_data)
    read_data = disk2.read_sector(0, 1)
    print(f"Write/Read test: {'PASS' if read_data == test_data else 'FAIL'}")
