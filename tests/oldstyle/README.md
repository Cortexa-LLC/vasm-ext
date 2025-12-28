# Oldstyle Syntax Module Tests

Test files for vasm's oldstyle syntax module and general assembler features.

## Overview

These tests verify basic vasm functionality and oldstyle syntax features. Many tests were created during development and debugging of various vasm features, including case sensitivity, label handling, and directive parsing.

## Running Tests

From the vasm root directory:

```bash
# Build the oldstyle assembler
make CPU=6502 SYNTAX=oldstyle

# Run a single test
./vasm6502_oldstyle -Fbin -o tests/oldstyle/test_name.bin tests/oldstyle/test_name.s

# Run all oldstyle tests
for test in tests/oldstyle/test_*.s; do
    echo "Testing: $test"
    ./vasm6502_oldstyle -Fbin -o "${test%.s}.bin" "$test" || echo "FAILED: $test"
done
```

## Test Categories

### Case Sensitivity Tests

**test_case_insensitive.s**
- Tests case-insensitive assembly with `-nocase` flag
- Verifies label matching regardless of case

**test_mixed_case.s**
- Tests mixed-case labels and mnemonics
- Verifies case handling in identifiers

**test_nocase_backward.s**
- Tests backward references with `-nocase`
- Verifies case-insensitive label lookup

**test_nocase_forward.s**
- Tests forward references with `-nocase`
- Verifies case-insensitive forward declarations

**test_nocase_issue.s**
- Specific test case for case-insensitivity bugs
- Regression test for `-nocase` flag issues

**test_nocase_only.s**
- Tests `-nocase` without other features
- Minimal test for case-insensitive mode

### Label Tests

**test_label.s**
- Basic label definition and reference test
- Verifies fundamental label functionality

**test_label2.s**
- Additional label test cases
- Tests label variations

**test_simple_label.s**
- Minimal label test
- Simple label definition and usage

**test_oldstyle_colon.s**
- Tests labels with colons (e.g., `START:`)
- Verifies colon suffix on labels

**test_oldstyle_no_colon.s**
- Tests labels without colons
- Verifies optional colon syntax

### Directive Tests

**test_db.s**
- Tests DB (define byte) directive
- Verifies byte data definition

**test_dc.s**
- Tests DC (define constant) directive
- Verifies constant definition

**test_end.s**
- Tests END directive
- Verifies end-of-assembly marker

**test_no_end.s**
- Tests assembly without END directive
- Verifies optional END directive

**test_refs.s**
- Tests symbol references
- Verifies forward/backward reference handling

### Comment Tests

**test_column1.s**
- Tests comments in column 1
- Verifies comment handling at start of line

### Syntax Comparison Tests

**test_std_syntax.s**
- Tests standard syntax style (`.org` with lowercase)
- Compares with oldstyle syntax

**test_oldstyle_colon.s**
- Tests oldstyle with label colons
- Syntax variation test

### General Tests

**test_hello_copy.s**
- General assembly test
- Likely a "hello world" style test program

**test_issue.s**
- Specific bug reproduction test
- Regression test for a particular issue

**test_tabs.s**
- Tests tab character handling
- Verifies whitespace parsing

**test_ast.bin**, **test.bin**
- Pre-existing binary test files
- Reference outputs or binary data tests

## Syntax Features

The oldstyle syntax module supports:

- **Comments**: `;` semicolon comments (inline and full-line)
- **Labels**: Optional colon suffix (e.g., `LABEL:` or `LABEL`)
- **Directives**: Traditional uppercase (`ORG`, `DC`, `DB`, `END`, etc.)
- **Case handling**: Case-sensitive by default, `-nocase` for case-insensitive
- **Number formats**: `$hex`, `%binary`, decimal
- **Mnemonics**: Standard 6502 instruction set

## Example Syntax

```asm
; Oldstyle syntax example
    ORG $0C00

START:  LDA #$00
        STA $D020
        JSR Subroutine
        RTS

Subroutine:
        LDX #$0A
Loop    DEX
        BNE Loop
        RTS
```

## Notes

- These tests use oldstyle or general vasm syntax
- Some tests may require specific command-line flags (e.g., `-nocase`)
- Not all tests may pass without appropriate flags or modifications
- Tests marked with "issue" are typically regression tests for specific bugs
- Binary files (`.bin`) in this directory are reference outputs or test data

## Comparison with SCMASM Syntax

Key differences from SCMASM syntax:

| Feature | Oldstyle | SCMASM |
|---------|----------|--------|
| Comments | `;` | `*` (column 1), `;` (inline) |
| Directives | `ORG`, `DC`, `DB` | `.OR`, `.DA`, `.HS` |
| Label suffix | Optional `:` | No colon |
| Local labels | Not built-in | `.N` format |
| Private labels | Not built-in | `:N` format |
| Case | Sensitive (default) | Sensitive (default) |

## Adding Tests

When adding new oldstyle tests:

1. Use traditional oldstyle syntax (uppercase directives, optional colons)
2. Use semicolon comments
3. Name tests descriptively (e.g., `test_forward_refs.s`)
4. Document expected behavior in comments
5. Update this README with test description

## References

- vasm oldstyle syntax documentation: http://sun.hasenbraten.de/vasm/
- 6502 instruction set reference
- Traditional 6502 assembler manuals
