# Makefile Utilities for vasm Projects

This directory contains reusable Makefile includes and scripts for vasm-based projects.

## Files

### prodos.mk - ProDOS Disk Image Creation

Creates bootable ProDOS disk images using AppleCommander.

**Requirements:**
- Java runtime
- AppleCommander acx.jar

**Usage:**
```makefile
include path/to/vasm-ext/make/prodos.mk

# Override defaults if needed
PRODOS_TEMPLATE = prodos/blank140k.po
PRODOS_IMAGE = build/mydisk.po

# Build disk and add files
disk: prodos-disk
    $(AC) import -d $(PRODOS_IMAGE) --raw --stdin -t $(FTYPE_SYS) -a 0x2000 -n MYFILE < build/myfile.bin
```

**Targets:**
- `prodos-disk` - Create blank ProDOS disk from template
- `prodos-add-bin` - Add binary file (requires FILE, NAME, ADDR)
- `prodos-add-sys` - Add system file (requires FILE, NAME, ADDR)
- `prodos-list` - List disk contents
- `prodos-info` - Show disk information
- `prodos-map` - Show disk usage map
- `prodos-get` - Extract file (requires NAME, OUT)
- `prodos-delete` - Delete file (requires NAME)
- `prodos-clean` - Remove disk image
- `prodos-help` - Show help

### memory-map.py - Memory Map Generator (Recommended)

Python script that generates memory maps from vasm listing files. Parses listing
files to extract ORG addresses, sizes, and symbols. Supports JSON configuration
for complex multi-component builds with relocation.

**Requirements:**
- Python 3.6+

**Basic Usage (listing files only):**
```bash
python3 memory-map.py build/main.lst build/module.lst -o build/MEMORY.MAP
```

**Advanced Usage (JSON config):**
```bash
python3 memory-map.py -c memory-map.json -o build/MEMORY.MAP
```

**JSON Config Format:**
```json
{
    "title": "My Project Memory Map",
    "load_address": "0x2000",
    "components": [
        {
            "name": "Main Program",
            "listing": "build/main.lst",
            "binary": "build/main.bin",
            "run_address": "0x8000",
            "pad_to": "0x2200"
        },
        {
            "name": "Support Module",
            "listing": "build/support.lst",
            "binary": "build/support.bin",
            "pad_to": "0x4000",
            "run_address": "0xA000"
        }
    ],
    "runtime_regions": [
        {
            "name": "User data area",
            "start": "0x0800",
            "end": "0x1FFF"
        }
    ]
}
```

**Component Options:**
- `name` - Display name for the component
- `listing` - Path to vasm listing file (extracts ORG and size)
- `binary` - Path to binary file (uses file size if listing not available)
- `run_address` - Runtime address (if different from load address due to relocation)
- `pad_to` - Absolute load address (for padding calculations)
- `size_override` - Force a specific size instead of computed

**Command Line Options:**
- `-c, --config FILE` - JSON config file
- `-o, --output FILE` - Output file (default: stdout)
- `--max-symbols N` - Max symbols per listing (default: 50, 0=all)
- `--no-symbols` - Don't include symbol table

**Makefile Integration:**
```makefile
VASM_EXT = ../vasm-ext
MEMORY_MAP_PY = $(VASM_EXT)/make/memory-map.py

$(BUILD_DIR)/MEMORY.MAP: memory-map.json $(BUILD_DIR)/main.bin | $(BUILD_DIR)
    python3 $(MEMORY_MAP_PY) -c memory-map.json -o $@

.PHONY: memory-map
memory-map: $(BUILD_DIR)/MEMORY.MAP
```

### memory-map.mk - Simple Memory Map Generation (Alternative)

Makefile-only approach for simple projects without relocation. Extracts symbols
from listing files and generates a basic memory map.

**Requirements:**
- vasm with `-L` option for listing file generation

**Usage:**
```makefile
BUILD_DIR = build
LISTING_FILES = $(BUILD_DIR)/main.lst $(BUILD_DIR)/module.lst

include path/to/vasm-ext/make/memory-map.mk

# Add to your build target
all: myprogram
    @$(MAKE) --no-print-directory memory-map
```

**Targets:**
- `memory-map` - Generate memory map file
- `memory-map-symbols` - Append symbols to existing memory map
- `show-memory-map` - Generate and display memory map
- `memory-map-clean` - Remove memory map file
- `memory-map-help` - Show help

**Configuration:**
- `BUILD_DIR` - Build output directory (default: build)
- `MEMORY_MAP` - Output file path (default: $(BUILD_DIR)/MEMORY.MAP)
- `LISTING_FILES` - List of .lst files to process
- `MAX_SYMBOLS_PER_FILE` - Max symbols per listing (default: 50, 0=all)

## Example: SCASM Project Integration

```makefile
# Makefile for SCMASM (uses Python memory-map.py)

VASM = vasm6502_scmasm
VASM_EXT = ../vasm-ext
BUILD_DIR = build

# Include ProDOS support
include $(VASM_EXT)/make/prodos.mk

# Memory map using Python script
MEMORY_MAP_PY = $(VASM_EXT)/make/memory-map.py
MEMORY_MAP_CONFIG = memory-map.json

$(BUILD_DIR)/MEMORY.MAP: $(MEMORY_MAP_CONFIG) $(BUILD_DIR)/SCASM | $(BUILD_DIR)
    python3 $(MEMORY_MAP_PY) -c $(MEMORY_MAP_CONFIG) -o $@
    cat $@

# Build with listing files
$(BUILD_DIR)/SCASM: main.s
    $(VASM) -Fbin -L $(BUILD_DIR)/SCASM.lst -o $@ $<

# Main target
all: $(BUILD_DIR)/SCASM
    @$(MAKE) --no-print-directory $(BUILD_DIR)/MEMORY.MAP

# Create bootable disk
disk: all prodos-disk
    $(AC) import -d $(PRODOS_IMAGE) ... < $(BUILD_DIR)/SCASM
```

## Choosing Between memory-map.py and memory-map.mk

| Feature | memory-map.py | memory-map.mk |
|---------|---------------|---------------|
| Relocation support | Yes | No |
| Custom layouts | JSON config | Manual echo |
| Cross-platform | Yes (Python) | Unix-like (Make/grep) |
| Complexity | Medium | Simple |
| Best for | Multi-component builds | Single-file projects |
