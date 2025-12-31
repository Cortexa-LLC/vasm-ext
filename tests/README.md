# vasm Test Suite

This directory contains test suites for various vasm syntax modules and features.

## Directory Structure

```
tests/
├── README.md           # This file
├── scmasm/             # SCMASM syntax module tests
│   ├── README.md       # SCASM test documentation
│   └── test_*.s        # SCASM test files (features + original suite)
├── merlin/             # Merlin syntax module tests
│   ├── README.md       # Merlin test documentation
│   └── test_*.asm      # Merlin test files
└── oldstyle/           # Oldstyle syntax module tests
    └── test_*.s        # General/oldstyle test files
```

## Test Suites

### SCMASM Syntax Module Tests (`scmasm/`)

Tests for the S-C Macro Assembler (SCASM) 3.0/3.1 syntax module. These tests verify SCASM-specific features:

- Three-tier label system (global, private `:N`, local `.N`)
- SCASM directives (`.OR`, `.DA`, `.HS`, `.AS`, `.AZ`, `.AT`, `.OP`, etc.)
- Asterisk comments (`*` in column 1)
- Conditional assembly (`.DO`/`.ELSE`/`.FIN`)
- CPU type selection (`.OP`)
- Flexible string delimiters

**Building and running:**
```bash
make CPU=6502 SYNTAX=scmasm
./vasm6502_scmasm -Fbin -o tests/scmasm/test.bin tests/scmasm/test.s
```

**Documentation:** See `scmasm/README.md` for detailed test descriptions.

### Merlin Syntax Module Tests (`merlin/`)

Tests for the Merlin assembler syntax used in Apple II/IIgs development. Comprehensive coverage of all Merlin features:

- **CPU Variants**: 6502, 65C02, 65816 instruction sets
- **Label System**: Three-tier (global, `:LOCAL`, `]VARIABLE`) with scoping
- **Macro System**: MAC/<<<, ]0-]8 parameters, multiple invocation methods (direct, >>>, PMC)
- **File Imports**: PUT (source), USE (macros), PUTBIN (binary)
- **String Directives**: ASC, DCI, INV, FLS, REV, STR, STRL, HEX
- **CPU Modes**: MX, XC, LONGA, LONGI for 65816
- **Loops**: LUP/--^ directive
- **Directives**: Complete coverage of all Merlin directives
- **Apple IIgs**: Toolbox integration, OMF output

**Building and running:**
```bash
make CPU=6502 SYNTAX=merlin
./vasm6502_merlin -Fbin -o tests/merlin/test.bin tests/merlin/test.asm
```

**Documentation:** See `merlin/README.md` for detailed test descriptions.

### Oldstyle Syntax Module Tests (`oldstyle/`)

General tests and tests for the oldstyle syntax module. These tests use:

- Traditional assembly syntax (ORG, DC, DB, etc.)
- Semicolon comments (`;`)
- Optional label colons
- Standard directives
- Case sensitivity/insensitivity testing

**Building and running:**
```bash
make CPU=6502 SYNTAX=oldstyle
./vasm6502_oldstyle -Fbin -o tests/oldstyle/test.bin tests/oldstyle/test.s
```

## Running All Tests

### Using Make Targets (Recommended)

The Makefile includes convenient test targets:

```bash
# Test SCMASM syntax module
make test-scmasm

# Test oldstyle syntax module
make test-oldstyle

# Test both SCMASM and oldstyle
make test-all

# Test currently built CPU/SYNTAX combination
make CPU=6502 SYNTAX=scmasm test

# Alternative: use 'check' alias
make CPU=6502 SYNTAX=scmasm check
```

The test targets automatically:
- Build the assembler if needed
- Run all test files in the appropriate directory
- Report pass/fail status for each test
- Display summary statistics

### Manual Testing

#### SCMASM Tests
```bash
make CPU=6502 SYNTAX=scmasm
for test in tests/scmasm/test_*.s; do
    echo "Testing: $test"
    ./vasm6502_scmasm -Fbin -o "${test%.s}.bin" "$test" || echo "FAILED: $test"
done
```

#### Oldstyle Tests
```bash
make CPU=6502 SYNTAX=oldstyle
for test in tests/oldstyle/test_*.s; do
    echo "Testing: $test"
    ./vasm6502_oldstyle -Fbin -o "${test%.s}.bin" "$test" || echo "FAILED: $test"
done
```

## Test File Naming Convention

All test files follow the pattern `test_*.s`:

- `test_<feature>.s` - Tests for specific features
- `test_<syntax>_<feature>.s` - Syntax-specific feature tests

Binary outputs are generated as `test_*.bin` and are excluded from version control via `.gitignore`.

## Adding New Tests

When adding new tests:

1. Place test files in the appropriate subdirectory:
   - `scmasm/` for SCMASM-specific tests
   - `oldstyle/` for general or oldstyle syntax tests

2. Use descriptive test names (e.g., `test_macro_params.s`, `test_local_labels.s`)

3. Add test documentation to the subdirectory's README.md

4. Include expected output or byte count in comments if applicable

5. Test both successful assembly and error conditions where appropriate

## Test Output

Test binary outputs (`*.bin`) are automatically excluded by `.gitignore`. To verify test correctness:

- Check assembly completes without errors
- Compare binary size against expected byte count
- Use `hexdump -C` to inspect binary output
- Compare against reference binaries if available

## Notes

- Tests may produce warnings; check exit codes to verify success
- Some tests are designed to fail (error condition tests)
- Binary outputs may vary with vasm version or optimization level
- Test with `-Fbin` for simplicity unless testing specific output formats

## References

- vasm documentation: http://sun.hasenbraten.de/vasm/
- SCMASM documentation: https://www.txbobsc.com/scsc/scassembler/
- Source: https://github.com/burniouf/A2osX (SCMASM examples)
