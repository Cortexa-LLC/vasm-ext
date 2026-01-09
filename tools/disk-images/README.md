# TRS-80 Disk Image Tools

Tools for creating bootable NewDOS-80/TRS-DOS disk images with assembled /CMD programs.

## Overview

These tools help you:
1. Create blank TRS-80 disk images (DMK format)
2. Format disks for NewDOS-80 or TRS-DOS
3. Add /CMD executable files to disks
4. Test programs in TRS-80 emulators

## Requirements

### Emulators
- **trs80gp** (recommended) - http://48k.ca/trs80gp.html
- **sdltrs** - http://sdltrs.sourceforge.net/

### Python Dependencies
```bash
pip install -r requirements.txt
```

### System Files Needed
- NewDOS-80 system disk (NEWDOS.DSK or NEWDOS.JV3)
- Or TRS-DOS 6.x system disk

## Quick Start

### 1. Build Test Programs

```bash
# Build assembler
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80

# Assemble Phase 2 tests to /CMD format
cd tests/edtasm-m80/phase2
for test in test_*.asm; do
  ../../../vasmz80_edtasm-m80 -Fcmd -o ${test%.asm}.cmd $test
done
```

### 2. Create Bootable Disk

```bash
# Create blank disk
python tools/disk-images/create_disk.py --format newdos80 --output test_disk.dmk

# Add programs
python tools/disk-images/add_files.py test_disk.dmk tests/edtasm-m80/phase2/*.cmd

# List contents
python tools/disk-images/list_disk.py test_disk.dmk
```

### 3. Test in Emulator

```bash
# Using trs80gp
trs80gp -m3 -disk0 test_disk.dmk

# Using sdltrs
sdltrs -model 3 -disk0 test_disk.dmk
```

## Tools

### create_disk.py

Create blank formatted TRS-80 disk images.

```bash
# NewDOS-80 format (40 track, SSDD)
python create_disk.py --format newdos80 --output mydisk.dmk

# TRS-DOS 6 format (40 track, SSDD)
python create_disk.py --format trsdos6 --output mydisk.dmk

# Double-sided (80 track)
python create_disk.py --format newdos80 --tracks 80 --sides 2 --output mydisk.dmk
```

**Options:**
- `--format` - Disk format: newdos80, trsdos6, ldos
- `--tracks` - Number of tracks (35, 40, 80)
- `--sides` - Number of sides (1, 2)
- `--output` - Output DMK file

### add_files.py

Add /CMD files to disk images.

```bash
# Add single file
python add_files.py mydisk.dmk myprogram.cmd

# Add multiple files
python add_files.py mydisk.dmk *.cmd

# Specify TRS-80 filename (8.3 format)
python add_files.py mydisk.dmk myprogram.cmd --name TEST/CMD
```

**Options:**
- `--name` - TRS-80 filename (8.3 format, default: derived from file)
- `--overwrite` - Overwrite existing files

### list_disk.py

List contents of disk images.

```bash
# List all files
python list_disk.py mydisk.dmk

# Show details
python list_disk.py mydisk.dmk --verbose
```

### extract_file.py

Extract files from disk images.

```bash
# Extract single file
python extract_file.py mydisk.dmk TEST/CMD --output test.cmd

# Extract all files
python extract_file.py mydisk.dmk --all --output-dir extracted/
```

### make_bootable.py

Make disk bootable with NewDOS-80 or TRS-DOS.

```bash
# Copy system files from NewDOS-80 master
python make_bootable.py mydisk.dmk --system newdos80 --source NEWDOS.DSK

# Copy system files from TRS-DOS 6
python make_bootable.py mydisk.dmk --system trsdos6 --source TRSDOS.DSK
```

## Disk Formats

### DMK (David M. Keil Format)

Most compatible format, supported by all modern emulators.

**Structure:**
- Header: 16 bytes
- Track data: Variable size per track
- Supports: Single/double sided, single/double density

**Advantages:**
- Most compatible with emulators
- Preserves low-level disk structure
- Handles copy protection

### JV3 (Jeff Vavasour v3)

Simple format, widely supported.

**Structure:**
- Sector directory
- Sector data

**Advantages:**
- Simple structure
- Small file size
- Fast access

## Example Workflow

### Create Test Disk for Phase 2

```bash
#!/bin/bash
# create_test_disk.sh

# 1. Build assembler
make CPU=z80 SYNTAX=edtasm-m80

# 2. Assemble all Phase 2 tests
cd tests/edtasm-m80/phase2
for test in test_*.asm; do
  ../../../vasmz80_edtasm-m80 -Fcmd -o ${test%.asm}.cmd $test
done
cd ../../..

# 3. Create disk image
python tools/disk-images/create_disk.py \
  --format newdos80 \
  --output phase2_tests.dmk

# 4. Make it bootable (if you have NewDOS-80 master)
if [ -f "$HOME/trs80/NEWDOS.DSK" ]; then
  python tools/disk-images/make_bootable.py \
    phase2_tests.dmk \
    --system newdos80 \
    --source "$HOME/trs80/NEWDOS.DSK"
fi

# 5. Add all test programs
python tools/disk-images/add_files.py \
  phase2_tests.dmk \
  tests/edtasm-m80/phase2/*.cmd

# 6. List contents
python tools/disk-images/list_disk.py phase2_tests.dmk

echo "Disk image created: phase2_tests.dmk"
echo "To test: trs80gp -m3 -disk0 phase2_tests.dmk"
```

## Testing in Emulators

### trs80gp

```bash
# Model III, NewDOS-80
trs80gp -m3 -disk0 phase2_tests.dmk

# Model 4, TRS-DOS 6
trs80gp -m4 -disk0 phase2_tests.dmk
```

**Commands in emulator:**
```
DIR                     ; List files
DIR (ALL=YES)          ; List all files including system
<filename>             ; Run program (e.g., TEST_CMD_BASIC)
```

### sdltrs

```bash
# Model III
sdltrs -model 3 -disk0 phase2_tests.dmk

# Model 4
sdltrs -model 4 -disk0 phase2_tests.dmk
```

## Creating Demo Disk

Create a disk with simple demo programs:

```bash
# Create demo source
cat > tests/edtasm-m80/demo.asm << 'EOF'
; Simple TRS-80 Demo
; Displays "Hello, TRS-80!" and waits for key

  ORG $8000

; TRS-80 Model III video memory
VIDEO EQU $3C00

START:
  LD HL,VIDEO
  LD DE,MESSAGE
  LD B,14

LOOP:
  LD A,(DE)
  LD (HL),A
  INC HL
  INC DE
  DJNZ LOOP

WAIT:
  CALL $0049        ; Key scan routine
  JR Z,WAIT
  RET

MESSAGE:
  DEFB "HELLO, TRS-80!"

  END START
EOF

# Assemble
./vasmz80_edtasm-m80 -Fcmd -o tests/edtasm-m80/demo.cmd tests/edtasm-m80/demo.asm

# Create disk
python tools/disk-images/create_disk.py --format newdos80 --output demo.dmk
python tools/disk-images/add_files.py demo.dmk tests/edtasm-m80/demo.cmd --name DEMO/CMD

# Test
trs80gp -m3 -disk0 demo.dmk
# In emulator: type "DEMO" and press Enter
```

## Troubleshooting

### "File Not Found" in Emulator
- Verify file was added: `python list_disk.py mydisk.dmk`
- Check filename format (8.3, uppercase)
- Ensure disk is bootable

### "Error Loading" in Emulator
- Verify /CMD format: `hexdump -C program.cmd`
- Check for proper load blocks (0x01) and transfer block (0x02)
- Verify little-endian encoding

### Disk Won't Boot
- Need NewDOS-80 or TRS-DOS system files
- Use `make_bootable.py` to copy system files
- Or create disk in emulator and export

## References

### TRS-80 Disk Formats
- DMK Format: http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/dmk-format/
- JV3 Format: http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/jv3-format/

### Emulators
- trs80gp: http://48k.ca/trs80gp.html
- sdltrs: http://sdltrs.sourceforge.net/

### TRS-80 Resources
- TRS-80 Model III/4 Technical Reference
- NewDOS-80 Manual
- TRS-DOS 6 Manual

---

**Created:** 2026-01-09
**Status:** Tools in development
