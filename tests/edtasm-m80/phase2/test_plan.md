# Phase 2 Comprehensive Test Plan

## Test Coverage Goals

- **40-50 total tests** covering all Phase 2 features
- **100% feature coverage** for implemented features
- **Edge case testing** for robustness
- **Integration testing** for feature combinations

## Test Categories

### 1. Macro Parameters (8 tests)
- [x] test_macro_params.asm - Basic #P1-#P9 substitution
- [x] test_macro_unique_id.asm - #$YM unique IDs
- [ ] test_macro_params_all9.asm - All 9 parameters
- [ ] test_macro_params_nested.asm - Nested macro calls
- [ ] test_macro_params_case.asm - Case-insensitive parameters
- [ ] test_macro_params_empty.asm - Missing parameters
- [ ] test_macro_params_expr.asm - Parameters with expressions
- [ ] test_macro_params_strings.asm - String parameters

### 2. INCLUDE Directive (6 tests)
- [x] test_include_basic.asm - Basic include
- [x] test_include_nested.asm - Nested includes
- [ ] test_include_paths.asm - Include with -I paths
- [ ] test_include_missing.asm - Missing file error
- [ ] test_include_circular.asm - Circular include detection
- [ ] test_include_macro.asm - Include files with macros

### 3. REPT Directive (6 tests)
- [x] test_rept.asm - Basic repetition and nesting
- [ ] test_rept_zero.asm - Zero repetitions
- [ ] test_rept_expr.asm - Expression-based count
- [ ] test_rept_large.asm - Large repetition count
- [ ] test_rept_macro.asm - REPT inside macros
- [ ] test_rept_nested_deep.asm - Deep nesting

### 4. IRP Directive (6 tests)
- [x] test_irp.asm - Basic list iteration
- [ ] test_irp_single.asm - Single item
- [ ] test_irp_empty.asm - Empty list
- [ ] test_irp_nested.asm - Nested IRP
- [ ] test_irp_macro.asm - IRP inside macros
- [ ] test_irp_complex.asm - Complex list items

### 5. IRPC Directive (6 tests)
- [x] test_irpc.asm - Basic character iteration
- [ ] test_irpc_empty.asm - Empty string
- [ ] test_irpc_single.asm - Single character
- [ ] test_irpc_nested.asm - Nested IRPC
- [ ] test_irpc_macro.asm - IRPC inside macros
- [ ] test_irpc_special.asm - Special characters

### 6. EXITM Directive (4 tests)
- [x] test_exitm.asm - Unconditional exit
- [ ] test_exitm_conditional.asm - Conditional exit
- [ ] test_exitm_nested.asm - Exit from nested macro
- [ ] test_exitm_rept.asm - EXITM in REPT block

### 7. Integration Tests (8 tests)
- [x] test_macro_include_combined.asm - Macros + includes
- [ ] test_rept_irp_combined.asm - REPT + IRP
- [ ] test_macro_rept_combined.asm - Macros + REPT
- [ ] test_all_features.asm - All Phase 2 features
- [ ] test_complex_nesting.asm - Deep feature nesting
- [ ] test_os9_style.asm - OS-9 style code
- [ ] test_library.asm - Macro library pattern
- [ ] test_large_project.asm - Large realistic project

### 8. TRS-DOS /CMD Output (6 tests)
- [x] test_cmd_basic.asm - Single segment
- [x] test_cmd_multiseg.asm - Multiple segments
- [x] test_cmd_segments.asm - CSEG/DSEG
- [ ] test_cmd_large.asm - Large program
- [ ] test_cmd_gaps.asm - Non-contiguous segments
- [ ] test_cmd_boundary.asm - Address boundary cases

## Test Execution

### Build Command
```bash
make CPU=z80 SYNTAX=edtasm-m80
```

### Run Single Test
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin test_file.asm
./vasmz80_edtasm-m80 -Fcmd -o test.cmd test_file.asm
```

### Run All Tests
```bash
cd tests/edtasm-m80/phase2
for test in test_*.asm; do
  echo "Testing: $test"
  ../../../vasmz80_edtasm-m80 -Fbin -o ${test%.asm}.bin $test && echo "  PASS" || echo "  FAIL"
done
```

## Success Criteria

- [ ] All 40+ tests pass
- [ ] 100% feature coverage
- [ ] No compiler warnings
- [ ] No memory leaks
- [ ] Documentation complete

## Current Status

- **Tests Created:** 13/40+
- **Tests Passing:** 13/13 (100%)
- **Coverage:** ~50% (core features covered)
