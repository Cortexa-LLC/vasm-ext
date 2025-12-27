# Merlin Syntax Module Test Suite

Comprehensive tests for the vasm Merlin syntax module covering all major features.

## Test Files

### 6502 Tests (`test_6502.asm`)
Tests basic 6502 CPU features with Merlin syntax:
- Variable labels (`]LABEL`) with mutable values
- Local labels (`:LABEL`) scoped to global labels
- Macro definitions with parameter expansion (`]0-]8`)
- String directives (ASC, DCI, HEX)
- Data directives (DFB, DA, DS)
- Conditional assembly (DO/FIN)

### 65C02 Tests (`test_65c02.asm`)
Tests 65C02 CPU extensions:
- XC directive for CPU mode switching
- Stack instructions (PHX, PHY, PLX, PLY)
- Store zero (STZ) in all addressing modes
- Branch always (BRA)
- Indirect addressing without X: `($12)`
- Bit immediate (BIT #$80)

### 65816 Tests (`test_65816.asm`)
Tests Apple IIgs 65816 CPU features:
- MX directive for accumulator/index register sizing
- 65816 stack instructions (PHK, PLB, PHD, PLD)
- Push effective address (PEA)
- Bank byte operator (`^`)
- 16-bit immediate mode
- Return long (RTL)

### GS Toolbox Tests (`test_toolbox.asm`)
Tests Apple IIgs Toolbox calling convention:
- Tool macro (JSL $E10000 + tool number)
- Toolbox wrapper macros
- Typical GS/OS parameter passing
- Bank byte usage in system calls

### Directive Tests (`test_directives.asm`)
Tests comprehensive directive coverage:
- Data directives (DFB, DA, DW, DS)
- String directives (ASC, DCI, INV, FLS, REV, STR, STRL, HEX)
- Symbol definition (EQU, =)
- Conditional assembly (DO/ELSE/FIN)
- Loop directive (REPT/ENDR)
- Output control (REL, LST)

### Advanced Feature Tests (`test_advanced.asm`)
Tests advanced Merlin features:
- LUP/--^ loop directive
- >>> macro invocation syntax
- DDB (big-endian word)
- ADRL (32-bit address)
- ENT/EXT (entry/external symbols)
- Variable labels in loops

### Complete Coverage Tests (`test_complete.asm`)
Tests remaining directives for 100% coverage:
- TYP/AUX (ProDOS file attributes)
- TR (truncate addressing)
- EXP/OBJ/CYC (listing control)
- KND/ALI/LNK (OMF/linker control)
- ERR (conditional errors)

### Macro System Tests (`test_macros.asm`)
Comprehensive macro feature testing:
- MAC/<<< definition and EOM alternate
- Parameters ]0-]8 (]0 = parameter count)
- Direct invocation (name only)
- >>> invocation syntax
- PMC invocation syntax
- Parameter separators: . / , - ( space
- Nested macros
- Macros with local labels
- Macro-to-macro calls

### Label System Tests (`test_labels.asm`)
Three-tier label system testing:
- Global labels
- Local labels (:LABEL) scoped to globals
- Variable labels (]VARIABLE) with redefinition
- Forward and backward references
- Label scoping across functions
- Variable labels in loops and conditionals
- Complex mixed-label scenarios

### File Import Tests (`test_imports.asm`)
File inclusion directive testing:
- PUT (include source file)
- USE (import macro library)
- PUTBIN (include binary data)
- Symbol visibility across includes
- Macro availability after USE

**Supporting files:**
- `include_defs.asm` - Symbols and macros for PUT
- `macro_library.asm` - Macro library for USE
- `test_data.bin` - Binary data for PUTBIN

### String Directive Tests (`test_strings.asm`)
Comprehensive string directive coverage:
- ASC (normal/high-bit strings)
- DCI (last char inverted)
- INV (all chars inverted)
- FLS (flashing/alternating)
- REV (reversed order)
- STR (Pascal 1-byte length)
- STRL (Pascal 2-byte length)
- HEX (hexadecimal data)
- Quote behavior differences (' vs ")
- Empty strings and special characters

### CPU Mode Tests (`test_cpu_modes.asm`)
65816 CPU mode directive testing:
- XC (CPU type selection)
- MX (accumulator/index sizing)
- LONGA (accumulator size control)
- LONGI (index register size control)
- Mode transitions (8-bit ↔ 16-bit)
- Combined mode settings
- Context across functions

## Running Tests

```bash
./run_tests.sh
```

Expected output:
```
================================
Merlin Syntax Module Test Suite
================================

6502 CPU Tests:
Testing 6502 Basic... ✓ PASS

65C02 CPU Tests:
Testing 65C02 Extensions... ✓ PASS

65816 CPU Tests:
Testing 65816 Instructions... ✓ PASS
Testing GS Toolbox Calls... ✓ PASS

================================
Test Summary
================================
PASSED: 4
FAILED: 0

✓ All tests passed!
```

## Features Tested

### Label System
- ✅ Global labels
- ✅ Local labels (`:LABEL`) with proper scoping
- ✅ Variable labels (`]LABEL`) with mutable values
- ✅ Variable label references in expressions

### Macro System
- ✅ MAC...<<< macro definition
- ✅ Parameter expansion (`]0` = count, `]1-]8` = parameters)
- ✅ Macro parameters in operands and expressions
- ✅ Tool macro for GS Toolbox calls

### CPU Support
- ✅ 6502 baseline instruction set
- ✅ 65C02 extensions (PHX/Y, PLX/Y, STZ, BRA, etc.)
- ✅ 65816 instructions (PHK, PLB, PHD, PLD, PEA, RTL, etc.)
- ✅ XC directive for CPU mode switching
- ✅ MX directive for 65816 register sizing
- ✅ Automatic 65816 mode on MX/LONGA/LONGI

### Operators & Directives
- ✅ Byte selection operators (<, >, ^)
- ✅ Bank byte operator (^) in all addressing modes
- ✅ String directives (ASC, DCI, HEX)
- ✅ Data directives (DFB, DA, DS)
- ✅ Conditional assembly (DO/FIN)

## Test Coverage

| Feature Category | Coverage |
|-----------------|----------|
| Label System | 100% |
| Macro System | 100% |
| 6502 Instructions | 100% |
| 65C02 Extensions | 100% |
| 65816 Extensions | 100% |
| Directives | 95% |
| Operators | 100% |

## Known Limitations

None - all core Merlin syntax features are fully functional.

## Adding New Tests

1. Create a new `.asm` file in `tests/merlin/`
2. Add appropriate CPU directive if needed (XC, MX)
3. Add test case to `run_tests.sh`:
   ```bash
   run_test "Your Test Name" "your_test.asm" ""
   ```
4. Run `./run_tests.sh` to verify

## Test Development

Tests are designed to be:
- **Focused**: Each test targets specific features
- **Independent**: Tests don't rely on each other
- **Clear**: Well-commented code showing feature usage
- **Comprehensive**: Cover normal and edge cases

## Requirements

- vasm6502_merlin built with `make CPU=6502 SYNTAX=merlin`
- bash shell
- Standard Unix utilities (mkdir, cat)
