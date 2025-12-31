# SCMASM Syntax Module Tests

Test suite for the SCMASM (S-C Macro Assembler) 3.0/3.1 syntax module.

For SCMASM syntax documentation, see `/syntax/scmasm/README.md`.

## Running Tests

From the vasm root directory:

```bash
# Build the SCMASM assembler
make CPU=6502 SYNTAX=scmasm

# Run a single test
./vasm6502_scmasm -Fbin -o tests/scmasm/test_name.bin tests/scmasm/test_name.s

# Run all SCMASM tests
for test in tests/scmasm/test_*.s; do
    echo "Testing: $test"
    ./vasm6502_scmasm -Fbin -o "${test%.s}.bin" "$test" || echo "FAILED: $test"
done
```

## Test Files

### Phase 1: Basic Assembly (MVP)

**test_scmasm.s** - Basic SCMASM syntax test
- Tests: global labels, `.OR` (origin), `.DA` (data), `.HS` (hex string)
- Verifies fundamental directive recognition and code generation
- Expected output: 14 bytes

**test_global_only.s** - Global labels only
- Tests global labels without local/private labels
- Verifies basic label definition and reference
- Expected output: 9 bytes

### Phase 2: Three-Tier Label System

**test_private_simple.s** - Simple private label test
- Tests private labels `:1` within a single function context
- Verifies basic private label scoping
- Expected output: 8 bytes

**test_private_labels.s** - Comprehensive private label test
- Tests `:N` labels in multiple function contexts (FUNC1, FUNC2)
- Verifies that `:1` in FUNC1 is distinct from `:1` in FUNC2
- Demonstrates 16-bit context scoping

**test_labels_simple.s** - Simple local label test
- Tests local labels `.1`, `.2` scoped to global label
- Verifies basic local label scoping
- Expected output: 17 bytes

**test_labels.s** - Extended label system test
- Tests complex label interactions and edge cases
- Comprehensive validation of label scoping rules

**test_all_labels.s** - All three label types together
- Tests global labels (FUNC1, FUNC2, MAIN, LOOP)
- Tests private labels `:1`, `:2` in different function contexts
- Tests local labels `.1`, `.2` scoped to different global labels
- Verifies all label types work correctly in combination
- Expected output: 36 bytes
- **Hex dump verification:**
  ```
  a2 00 e8 e0 05 d0 fb 60  a2 10 ca e0 0b d0 fb 60
  20 00 80 20 08 80 a0 00  c8 c0 03 d0 fb 60 a2 08
  ca d0 fe 60
  ```
  - Branch offsets confirm correct label resolution

### Phase 2: Conditionals

**test_conditionals.s** - Conditional assembly
- Tests `.DO`, `.ELSE`, `.FIN` conditional assembly
- Tests with `.EQ` constant definitions (DEBUG=1, FAST=0)
- Verifies conditional code inclusion/exclusion
- Expected output: 9 bytes
- **Expected behavior:**
  - DEBUG=1 includes `LDA #$FF`, excludes NOPs
  - FAST=0 excludes NOPs, includes `LDA #$01`

### Phase 3: Data Directives and CPU Selection

**test_op_directive.s** - CPU type selection
- Tests `.OP` directive for switching CPU types mid-assembly
- Switches between 6502, 65C02, and 65816
- Verifies CPU-specific instructions assemble correctly
- Expected output: 11 bytes
- **Hex dump verification:**
  ```
  ea a9 00 8d 20 d0 64 00 c2 30 60
  ```
  - `ea` = NOP (6502)
  - `a9 00` = LDA #$00 (6502)
  - `8d 20 d0` = STA $D020 (6502)
  - `64 00` = STZ $00 (65C02 instruction, opcode $64)
  - `c2 30` = REP #$30 (65816 instruction, opcode $C2)
  - `60` = RTS

**test_phase3.s** - Comprehensive Phase 3 test
- Tests all Phase 3 directives together
- Expected output: 61 bytes
- **Features tested:**
  - `.EQ` - Equate/constant definitions
  - `.AS` - ASCII strings with flexible delimiters (`/TEXT/`, `"TEXT"`, `'TEXT'`)
  - `.AZ` - ASCII zero-terminated strings
  - `.AT` - ASCII with high bit set on last character
  - `.OP` - CPU type selection (6502/65C02/65816)
  - `.DO`/`.FIN` - Conditional assembly
- **Hex dump verification:**
  ```
  44 65 62 75 67 20 4d 6f 64 65 ea a9 ff 8d 20 d0
  48 65 6c 6c 6f 57 6f 72 6c 64 00 54 65 73 f4 64
  00 9c 20 d0 c2 30 e2 20 60 41 53 43 49 49 5a 65
  72 6f 2d 74 65 72 6d 00 48 69 42 69 f4
  ```
  - Includes "Debug Mode" string (conditional assembly)
  - "Hello" without terminator (.AS)
  - "World\0" with zero terminator (.AZ)
  - "Test" with high bit on last 't' = $F4 (.AT)
  - STZ, REP, SEP CPU-specific instructions

### Phase 4: Macro System

**test_macro_simple.s** - Basic macro definition and invocation
- Tests macro definition with `.MA` and `.EM`
- Tests both `>MACRONAME` (traditional) and `_MACRONAME` (SCMASM 3.0) invocation
- Simple macros without parameters
- Expected output: 17 bytes
- **Features tested:**
  - `.MA`/`.EM` macro definition
  - `>MACRONAME` invocation
  - `_MACRONAME` invocation
  - Macro expansion into instructions

**test_macro_params.s** - Macro parameters
- Tests macro parameters `]1`-`]9`
- Tests parameter count `]#`
- Tests escape sequence `]]` for single `]`
- Multiple parameter passing
- Expected output: varies
- **Features tested:**
  - `]1` - Single parameter substitution
  - `]1`, `]2` - Multiple parameter substitution
  - `]1`, `]2`, `]3` - Three parameters
  - `]#` - Parameter count expansion
  - `]]` - Escape to single `]` character

**test_macro_invoke.s** - Both invocation styles
- Comprehensive test of `>` and `_` prefix invocation
- Tests multiple macro definitions
- Tests macros with and without parameters
- Expected output: varies
- **Features tested:**
  - Mixed invocation styles in same file
  - Multiple macro definitions
  - Parameter passing with both styles
  - Sequential macro invocations

### Phase 5: Phase Assembly and Dummy Sections

**test_phase.s** - Phase assembly (.PH/.EP)
- Tests `.PH` (phase start) and `.EP` (end phase) directives
- Code assembled at one address but executed at another
- Expected output: 21 bytes
- **Features tested:**
  - `.PH <address>` - Start phase assembly
  - `.EP` - End phase assembly
  - Address calculation during phase
  - Private labels within phased code
  - Multiple phase blocks in same file

**test_dummy.s** - Dummy sections (.DUMMY/.ED)
- Tests `.DUMMY` and `.ED` directives for structure definitions
- Defines data layouts without allocating space
- Expected output: 14 bytes
- **Features tested:**
  - `.DUMMY` - Start dummy section
  - `.ED` - End dummy section
  - Multiple dummy sections in same file
  - Label references to dummy section symbols
  - `.BS` (block storage) in dummy sections

### Phase 6: Edge Case Tests

**test_nested_conditionals.s** - Deeply nested conditionals
- Tests conditional assembly with up to 10 levels of nesting
- Verifies `.DO`/.ELSE`/.FIN` nesting works correctly
- Tests alternating TRUE/FALSE conditions at each level
- Expected output: 26 bytes
- **Features tested:**
  - Deep nesting (10 levels demonstrated, supports up to 63)
  - `.DO` with constant expressions
  - `.ELSE` clauses at various nesting levels
  - `.FIN` properly closing blocks
  - Code inclusion/exclusion based on conditions

**test_macro_complex.s** - Comprehensive macro features
- Tests all macro capabilities in combination
- Expected output: 48 bytes
- **Features tested:**
  - `]#` - Parameter count feature
  - `]1`-`]9` - All 9 parameter positions
  - `]]` - Escape sequence in comments
  - `.DO` conditionals inside macros
  - Both `>` and `_` invocation styles
  - Private labels within macros (single invocation)
  - Empty macros (edge case)

**test_label_scoping.s** - Complex label scoping
- Tests interactions between all three label types
- Verifies scoping rules in complex scenarios
- Expected output: 148 bytes
- **Features tested:**
  - Global, private, and local labels mixed together
  - Multiple local labels (`.1` through `.10`) in same function
  - Multiple private labels (`:1`, `:2`, `:3`) in same context
  - Label reuse across different scopes
  - Forward and backward references
  - Global label references across functions
  - Context switching (private labels reset on global labels)
  - Same-numbered labels in different contexts

### A2osX Compatibility

**test_a2osx_directives.s** - A2osX editor directives
- Tests NEW, AUTO, .TF, and TEXT directives
- Verifies metadata extraction for build automation
- Expected output: 22 bytes
- **Features tested:**
  - `NEW` - Editor command (ignored)
  - `AUTO 3,1` - Automatic line numbering (parsed and ignored)
  - `.TF BIN/TEST` - Target binary path extraction
  - `TEXT SRC/TEST.S` - Source listing path extraction
  - Full A2osX-style source file structure
  - Metadata displayed during assembly

## Test Coverage Summary

### Implemented and Tested ✓

- [x] Comments (`*` in column 1, `;` inline)
- [x] Global labels (MAIN, FUNC1, etc.)
- [x] Private labels (`:1`, `:2`, `:N`) - 16-bit context scope
- [x] Local labels (`.1`, `.2`, `.N`) - scoped to global label
- [x] `.OR` - Origin directive
- [x] `.DA` - Define address/word data
- [x] `.HS` - Hex string (with flexible separators)
- [x] `.BS` - Block storage (reserve bytes)
- [x] `.DO`/`.ELSE`/`.FIN` - Conditional assembly
- [x] `.EQ` - Equate/constant definition
- [x] `.AS` - ASCII string (flexible delimiters)
- [x] `.AZ` - ASCII zero-terminated (flexible delimiters)
- [x] `.AT` - ASCII with high bit on last char (flexible delimiters)
- [x] `.OP` - CPU type selection (6502/65C02/65R02/65816)
- [x] `.MA`/`.EM` - Macro definition and end
- [x] `>MACRONAME` - Macro invocation (traditional style)
- [x] `_MACRONAME` - Macro invocation (SCMASM 3.0 style)
- [x] `]1`-`]9` - Macro parameter substitution
- [x] `]#` - Macro parameter count
- [x] `]]` - Escape sequence for single `]`
- [x] `.PH`/`.EP` - Phase assembly (RORG/REND)
- [x] `.DUMMY`/`.ED` - Dummy sections (unallocated space)
- [x] `.INB` - Include binary files
- [x] `.LIST` - Listing control (inherited from oldstyle)

### Pending (Future Phases)
- [ ] `.CS`/`.CZ`/`.PS` - Additional string formats
- [ ] `.TF` - Target file metadata

## Validating Test Results

### Check Assembly Success

```bash
./vasm6502_scmasm -Fbin -o test.bin test.s
echo $?  # Should be 0 for success
```

### Verify Byte Count

```bash
ls -l test.bin
# Compare with expected byte count from test description
```

### Inspect Binary Output

```bash
hexdump -C test.bin
# Compare with expected hex values
```

### Verify with Disassembly

```bash
# For 6502 code, use a 6502 disassembler
# Compare disassembly with source to verify correct encoding
```

## Adding New Tests

When adding new SCMASM tests:

1. **Name:** Use `test_<feature>.s` naming convention
2. **Header:** Include comment describing test purpose
3. **Features:** Test one primary feature with supporting features
4. **Comments:** Document expected output and byte count
5. **Verification:** Include hex dump expectations if relevant
6. **Documentation:** Add test description to this README

Example test template:

```asm
* Test <feature> - Description
* Expected output: XX bytes
*
* This test verifies:
* - Feature A
* - Feature B interaction
*
        .OR $8000

* Test code here
START   NOP
        RTS
```

## Regression Testing

All `.bin` files in this directory are reference outputs. To verify:

1. Save current `.bin` files as reference
2. Make changes to syntax module
3. Rebuild and reassemble all tests
4. Compare new `.bin` files with saved references
5. Investigate any differences

```bash
# Save reference binaries
cp tests/scmasm/*.bin /tmp/scmasm_reference/

# After changes, rebuild and test
make CPU=6502 SYNTAX=scmasm clean && make CPU=6502 SYNTAX=scmasm
for test in tests/scmasm/test_*.s; do
    ./vasm6502_scmasm -Fbin -o "${test%.s}.bin" "$test"
done

# Compare
diff -r tests/scmasm/*.bin /tmp/scmasm_reference/
```

## Known Issues

None currently. All Phase 1-3 tests pass successfully.

## References

- **SCMASM syntax module:** `/syntax/scmasm/README.md`
- **SCMASM 2.0 documentation:** https://www.txbobsc.com/scsc/scassembler/SCMacroAssembler20.html
- **SCMASM 3.0 technical note:** https://www.txbobsc.com/scsc/scassembler/technote.3.0.html
- **A2osX project:** https://github.com/burniouf/A2osX (real-world SCMASM examples)
