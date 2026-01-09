# Testing with MAME TRS-80 Emulator

**Emulator Location:** `~/Projects/Vintage/TRS-80 Model III/MAME`

---

## MAME TRS-80 Support

MAME (Multiple Arcade Machine Emulator) includes excellent TRS-80 Model I/III/4 emulation.

### Supported Systems

- **trs80** - TRS-80 Model I (Level II BASIC)
- **trs80l2** - TRS-80 Model I (Level II BASIC, lowercase)
- **trs80m3** - TRS-80 Model III
- **trs80m4** - TRS-80 Model IV

### Disk Image Formats

MAME supports multiple TRS-80 disk formats:
- **DMK** - David M. Keil format (recommended, most compatible)
- **JV3** - Jeff Vavasour v3 format
- **DSK** - Simple disk format

Our tools create **DMK format** which is fully compatible with MAME.

---

## Quick Start

### 1. Build and Assemble Programs

```bash
# Build assembler
make CPU=z80 SYNTAX=edtasm-m80

# Create bootable test disk
./tools/disk-images/make_demo_disk.sh
```

### 2. Run in MAME

```bash
# Change to MAME directory
cd ~/Projects/Vintage/TRS-80\ Model\ III/MAME

# Run TRS-80 Model III with test disk
./mame trs80m3 -flop1 /path/to/vasm-ext/disk-images/phase2_tests.dmk
```

Or from vasm-ext directory:

```bash
# Set MAME path
MAME_DIR="$HOME/Projects/Vintage/TRS-80 Model III/MAME"

# Run with full path
"$MAME_DIR/mame" trs80m3 -flop1 "$PWD/disk-images/phase2_tests.dmk"
```

---

## MAME Command Line Options

### Basic Usage

```bash
# TRS-80 Model III with disk in drive 1
mame trs80m3 -flop1 mydisk.dmk

# TRS-80 Model I
mame trs80 -flop1 mydisk.dmk

# TRS-80 Model 4
mame trs80m4 -flop1 mydisk.dmk
```

### Advanced Options

```bash
# Run with two disk drives
mame trs80m3 -flop1 boot.dmk -flop2 data.dmk

# Run in window mode
mame trs80m3 -flop1 mydisk.dmk -window

# Run with specific NewDOS-80 version
mame trs80m3 -flop1 newdos80.dmk -flop2 mydisk.dmk

# Enable debugging
mame trs80m3 -flop1 mydisk.dmk -debug
```

### Useful MAME Keys

| Key | Function |
|-----|----------|
| Tab | MAME menu |
| F3 | Soft reset |
| Shift+F3 | Hard reset |
| F12 | Screenshot |
| Esc | Exit MAME |
| ~ | Speed throttle toggle |

---

## Testing Phase 2 Programs

### Method 1: Automated Script

```bash
# Create convenience script
cat > test_mame.sh << 'EOF'
#!/bin/bash
MAME_DIR="$HOME/Projects/Vintage/TRS-80 Model III/MAME"
DISK="$PWD/disk-images/phase2_tests.dmk"

echo "Starting MAME TRS-80 Model III..."
echo "Disk: $DISK"
echo ""
echo "In MAME, press Enter to boot, then type:"
echo "  DIR              - List files"
echo "  TEST_CMD_BASIC   - Run basic test"
echo ""

"$MAME_DIR/mame" trs80m3 -flop1 "$DISK" -window
EOF

chmod +x test_mame.sh
./test_mame.sh
```

### Method 2: Direct Command

```bash
# From vasm-ext directory
MAME_DIR="$HOME/Projects/Vintage/TRS-80 Model III/MAME"
"$MAME_DIR/mame" trs80m3 -flop1 "$PWD/disk-images/phase2_tests.dmk" -window
```

### Method 3: Create Alias

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
alias mame-trs80='$HOME/Projects/Vintage/TRS-80\ Model\ III/MAME/mame'
alias test-phase2='mame-trs80 trs80m3 -flop1 $PWD/disk-images/phase2_tests.dmk -window'
```

Then simply:

```bash
test-phase2
```

---

## Inside the Emulator

### Boot Sequence

1. MAME starts and shows TRS-80 Model III boot screen
2. Press **Enter** to boot from disk
3. NewDOS-80 or TRS-DOS loads (if disk is bootable)
4. You'll see the DOS prompt

### Running Programs

```
NEWDOS 80 V2.0
   01-01-80
   08:00 AM

:DIR                     <-- List files

NAME             PRI:TYP      Records

TEST_CMD_BASIC   00:CMD       00001
TEST_MACRO_PARAMS 00:CMD      00001
TEST_REPT        00:CMD       00001
...

:TEST_CMD_BASIC          <-- Run program
```

### Testing Checklist

For each test program:

1. **Load Program:**
   ```
   :TEST_CMD_BASIC
   ```

2. **Verify Execution:**
   - Program should load without errors
   - Check if program runs (may require keyboard input)
   - No crashes or hangs

3. **Check Output:**
   - For programs that write to screen
   - For programs that modify memory
   - For programs that return to DOS

---

## Common Issues

### Issue: "Bad Load Module"

**Cause:** /CMD file format incorrect

**Solutions:**
1. Verify with hexdump:
   ```bash
   hexdump -C program.cmd | head -3
   # Should show: 01 (load block) or 02 (transfer block)
   ```

2. Check little-endian encoding:
   ```bash
   # Address 0x8000 should be: 00 80 (little-endian)
   ```

3. Reassemble with verbose output:
   ```bash
   ./vasmz80_edtasm-m80 -Fcmd -v -o program.cmd program.asm
   ```

### Issue: Disk Won't Boot

**Cause:** No NewDOS-80/TRS-DOS system files

**Solutions:**
1. Use separate system disk:
   ```bash
   mame trs80m3 -flop1 newdos80.dmk -flop2 phase2_tests.dmk
   ```

2. Create bootable disk (requires NewDOS-80 master):
   ```bash
   python tools/disk-images/make_bootable.py \
     phase2_tests.dmk \
     --system newdos80 \
     --source ~/trs80/NEWDOS.DSK
   ```

### Issue: Files Not Visible

**Cause:** Directory not initialized or files not added properly

**Solutions:**
1. Recreate disk:
   ```bash
   ./tools/disk-images/make_demo_disk.sh
   ```

2. Verify disk contents:
   ```bash
   python tools/disk-images/list_disk.py phase2_tests.dmk
   ```

---

## Creating Test Scripts

### Simple Test Runner

```bash
#!/bin/bash
# test_with_mame.sh

MAME="$HOME/Projects/Vintage/TRS-80 Model III/MAME/mame"
TESTS_DIR="$PWD/tests/edtasm-m80/phase2"
DISK_DIR="$PWD/disk-images"

# Build assembler
echo "Building assembler..."
make CPU=z80 SYNTAX=edtasm-m80

# Assemble tests
echo "Assembling tests..."
cd "$TESTS_DIR"
for test in test_*.asm; do
    ../../../vasmz80_edtasm-m80 -Fcmd -Iinc -o "${test%.asm}.cmd" "$test"
done
cd -

# Create disk
echo "Creating disk..."
./tools/disk-images/make_demo_disk.sh

# Launch MAME
echo "Launching MAME..."
"$MAME" trs80m3 -flop1 "$DISK_DIR/phase2_tests.dmk" -window
```

### Automated Test Validation

```bash
#!/bin/bash
# validate_cmd_format.sh

echo "Validating /CMD file formats..."

for cmd in tests/edtasm-m80/phase2/test_*.cmd; do
    if [ -f "$cmd" ]; then
        echo -n "  $(basename "$cmd"): "

        # Check for load block (0x01) or transfer block (0x02)
        first_byte=$(hexdump -n 1 -e '1/1 "%02x"' "$cmd")

        if [ "$first_byte" = "01" ] || [ "$first_byte" = "02" ]; then
            echo "✓ VALID"
        else
            echo "✗ INVALID (first byte: 0x$first_byte)"
        fi
    fi
done
```

---

## MAME Configuration

### Recommended MAME Settings

Create `~/.mame/ini/trs80m3.ini`:

```ini
# TRS-80 Model III configuration for development

# Video
video                     opengl
window                    1
resolution                640x480
keepaspect                1

# Sound
sound                     auto
samplerate                48000

# Performance
autoframeskip             0
speed                     1.0

# Debugging
log                       0
verbose                   0

# Input
natural                   1
```

---

## Resources

### MAME TRS-80 Documentation
- MAME Manual: https://docs.mamedev.org/
- TRS-80 in MAME: https://www.mess.org/mess/trs80

### TRS-80 Resources
- TRS-80 Technical Reference
- NewDOS-80 Manual
- TRS-DOS 6 Manual

### Disk Image Tools
- DMK Format: http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/dmk-format/
- TRS-80 Emulation: http://www.trs-80.com/

---

## Quick Reference

### Common Commands

```bash
# Build and test in one command
make CPU=z80 SYNTAX=edtasm-m80 && \
  ./tools/disk-images/make_demo_disk.sh && \
  "$HOME/Projects/Vintage/TRS-80 Model III/MAME/mame" trs80m3 \
    -flop1 disk-images/phase2_tests.dmk -window

# Test specific program
./vasmz80_edtasm-m80 -Fcmd -o test.cmd myprogram.asm
python tools/disk-images/create_test_disk.py . disk.dmk
"$HOME/Projects/Vintage/TRS-80 Model III/MAME/mame" trs80m3 -flop1 disk.dmk

# Verify /CMD format
hexdump -C program.cmd | head -10
```

---

**Created:** 2026-01-09
**MAME Location:** `~/Projects/Vintage/TRS-80 Model III/MAME`
**Status:** Ready for testing
