# TRS-DOS /CMD Output Format

The TRS-DOS /CMD format is a block-based executable format used on TRS-80 Model I/III/4 computers running TRS-DOS.

## Format Specification

A /CMD file consists of one or more **Load Blocks** followed by a single **Transfer Block**.

### Load Block Structure

```
Offset  Size  Description
------  ----  -----------
0x00    1     Load block flag (0x01)
0x01    2     Block length including this header (little-endian)
0x03    2     Load address (little-endian)
0x05    N     Data bytes (N = block length - 5)
```

### Transfer Block Structure

```
Offset  Size  Description
------  ----  -----------
0x00    1     Transfer block flag (0x02)
0x01    2     Block length (always 0x0002)
0x03    2     Execution address (little-endian)
```

## Usage

### Basic Assembly

```bash
# Assemble to /CMD format
vasmz80_edtasm-m80 -Fcmd -o program.cmd program.asm
```

### Specify Entry Point

By default, the entry point is taken from the `END` directive or the first section's address.

```bash
# Specify custom entry point
vasmz80_edtasm-m80 -Fcmd -exec=START -o program.cmd program.asm
```

### Multi-Segment Programs

The /CMD output module automatically handles multiple sections, creating one load block per section:

```asm
  ORG $8000
CODE:
  LD A,$42
  RET

  ORG $9000
DATA:
  DEFB "Hello"

  END CODE
```

This produces:
1. Load block at 0x8000 (code section)
2. Load block at 0x9000 (data section)
3. Transfer block with entry at 0x8000

## Example Hexdump

Single-segment program:

```
00000000  01 0b 00 00 80 3e 55 21  34 12 c9 02 02 00 00 80
          ^^ Load block
             ^^^^^ Length (11 bytes)
                   ^^^^^ Address (0x8000)
                         ^^^^^^^^^^^^ Code
                                     ^^ Transfer block
                                        ^^^^^ Length (2)
                                              ^^^^^ Entry (0x8000)
```

## Segment Types

The /CMD format works with all segment types:

- **ASEG** - Absolute segment (fixed address)
- **CSEG** - Code segment
- **DSEG** - Data segment
- **ORG** - Origin directive

All segments are written as load blocks in address order.

## Testing

Test files in this directory:

- `test_cmd_basic.asm` - Single segment test
- `test_cmd_multiseg.asm` - Multiple ORG sections
- `test_cmd_segments.asm` - ASEG/CSEG/DSEG directives
- `test_cmd_exec.cmd` - Custom entry point test

## Compatibility

The /CMD format is compatible with:

- TRS-80 Model I/III/4 computers
- TRS-DOS 6.x
- LDOS
- LS-DOS
- Modern emulators (trs80gp, sdltrs)

## Implementation Details

**Source:** `output_cmd.c`

The module:
1. Sorts sections by start address
2. Skips empty and unallocated sections
3. Writes one load block per section
4. Writes a single transfer block with entry point
5. All values are little-endian (Z80 byte order)

**Copyright:** (c) 2026 Bryan Woodruff, Cortexa LLC
