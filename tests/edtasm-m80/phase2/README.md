# Phase 2 Test Suite - Z80 EDTASM-M80

**Phase 2:** Advanced Macros + Output (TRS-DOS /CMD format)
**Tests Created:** 2026-01-09
**Status:** Week 1 complete (5 tests)

---

## Test Organization

### Week 1 Tests (Macros + INCLUDE) - 5 tests
- **test_macro_params.asm** - Parameter substitution (#P1-#P9)
- **test_macro_unique_id.asm** - Unique ID generation (#$YM)
- **test_include_basic.asm** - Basic INCLUDE directive
- **test_include_nested.asm** - Nested INCLUDE directives
- **test_macro_include_combined.asm** - Macros + includes working together

### Week 2 Tests (Advanced Macros) - TODO
- test_local.asm - LOCAL directive
- test_rept.asm - REPT directive
- test_irp.asm - IRP directive
- test_irpc.asm - IRPC directive
- test_exitm.asm - EXITM directive
- test_concat.asm - & concatenation operator

### Week 3 Tests (/CMD Output) - TODO
- test_cmd_single.asm - Single-segment /CMD
- test_cmd_multi.asm - Multi-segment /CMD
- test_cmd_entry.asm - Entry point setting

---

## Running Tests

### Build assembler first:
```bash
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80
```

### Run individual test:
```bash
cd tests/edtasm-m80/phase2
../../../vasmz80_edtasm-m80 -Fbin -Iinc -o test.bin test_macro_params.asm
```

### Run all tests:
```bash
cd tests/edtasm-m80/phase2
for test in test_*.asm; do
  echo "=== Testing $test ==="
  ../../../vasmz80_edtasm-m80 -Fbin -Iinc -o ${test%.asm}.bin $test
  if [ $? -eq 0 ]; then
    echo "PASS"
  else
    echo "FAIL"
  fi
done
```

### View listing:
```bash
../../../vasmz80_edtasm-m80 -Fbin -Iinc -L test.lst test_macro_params.asm
cat test.lst
```

---

## Include Files (inc/ directory)

- **common.inc** - TRS-80 ROM routine addresses
- **trs80.inc** - TRS-80 screen/keyboard definitions (includes common.inc)
- **macros.inc** - Common macros (PUTCH, PRTSTR, CLS)

---

## Week 1 Test Coverage

### Macro Parameters (#P1-#P9)
✅ Single parameter substitution
✅ Multiple parameters (2, 3, 9)
✅ Case-insensitive (#P1 and #p1)
✅ Parameters in expressions
✅ Parameters in data directives

### Unique ID (#$YM)
✅ Basic unique label generation
✅ Multiple unique labels per macro
✅ Case-insensitive (#$YM and #$ym)
✅ Unique IDs across multiple invocations
✅ Nested macro invocations

### INCLUDE Directive
✅ Basic include from current directory
✅ Include from -I path
✅ Nested includes (2 levels)
✅ Symbol resolution across includes
✅ Macros in include files

---

## Expected Results

### test_macro_params.asm
- **Size:** 43 bytes
- **Features tested:** #P1-#P9, case-insensitive, 9 parameters max

### test_macro_unique_id.asm
- **Size:** 32 bytes
- **Features tested:** #$YM, unique labels, nested macros

### test_include_basic.asm
- **Size:** 12 bytes
- **Features tested:** Basic include, symbol resolution

### test_include_nested.asm
- **Size:** 14 bytes
- **Features tested:** Nested includes, 2-level depth

### test_macro_include_combined.asm
- **Size:** 37 bytes
- **Features tested:** Macros from includes, #P1 + #$YM combined

---

## Test Files Summary

```
phase2/
├── inc/                              # Include files
│   ├── common.inc                    # TRS-80 ROM addresses
│   ├── trs80.inc                     # Screen/keyboard (includes common.inc)
│   └── macros.inc                    # Common macros
├── test_macro_params.asm             # Week 1: #P1-#P9 tests
├── test_macro_unique_id.asm          # Week 1: #$YM tests
├── test_include_basic.asm            # Week 1: INCLUDE tests
├── test_include_nested.asm           # Week 1: Nested INCLUDE tests
├── test_macro_include_combined.asm   # Week 1: Combined tests
└── README.md                         # This file
```

**Total Week 1 tests:** 5
**All passing:** ✅ YES

---

## Next Steps (Week 2)

1. Implement LOCAL directive
2. Implement REPT directive
3. Implement IRP/IRPC directives
4. Implement EXITM directive
5. Implement & concatenation operator
6. Create tests for each feature

---

**Last Updated:** 2026-01-09
**Phase:** 2 Week 1 COMPLETE
**Tests Passing:** 5/5 (100%)
