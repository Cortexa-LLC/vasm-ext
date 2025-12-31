#!/usr/bin/env python3
"""
vasm Syntax Module Test Runner

Cross-platform test runner for vasm syntax modules.
Automatically discovers and runs tests for the specified syntax module.

Usage:
    python3 tests/run_tests.py <syntax> [assembler_path]

Examples:
    python3 tests/run_tests.py scmasm
    python3 tests/run_tests.py merlin ./vasm6502_merlin
"""

import os
import sys
import subprocess
import glob
import tempfile
import argparse
from pathlib import Path


# Test file patterns for each syntax module
SYNTAX_CONFIG = {
    'scmasm': {
        'test_dir': 'tests/scmasm',
        'pattern': 'test_*.s',
        'assembler': 'vasm6502_scmasm',
    },
    'merlin': {
        'test_dir': 'tests/merlin',
        'pattern': 'test_*.asm',
        'assembler': 'vasm6502_merlin',
    },
    'oldstyle': {
        'test_dir': 'tests/oldstyle',
        'pattern': 'test_*.s',
        'assembler': 'vasm6502_oldstyle',
    },
}


class TestResult:
    def __init__(self, name, passed, output="", error=""):
        self.name = name
        self.passed = passed
        self.output = output
        self.error = error


def find_project_root():
    """Find the project root directory (where Makefile is)."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / 'Makefile').exists() and (current / 'make.rules').exists():
            return current
        current = current.parent
    # Fallback to current working directory
    return Path.cwd()


def run_test(assembler_path, test_file, output_dir):
    """Run a single test file and return the result."""
    test_name = os.path.basename(test_file)
    output_file = os.path.join(output_dir, test_name + '.bin')

    try:
        result = subprocess.run(
            [assembler_path, '-Fbin', '-o', output_file, test_file],
            capture_output=True,
            text=True,
            timeout=30
        )

        # Check for errors in output
        output = result.stdout + result.stderr
        has_error = False
        error_lines = []

        for line in output.split('\n'):
            if line.startswith('error ') or line.startswith('fatal '):
                has_error = True
                error_lines.append(line)

        if has_error:
            return TestResult(test_name, False, output, '\n'.join(error_lines))
        else:
            return TestResult(test_name, True, output)

    except subprocess.TimeoutExpired:
        return TestResult(test_name, False, error="Test timed out after 30 seconds")
    except FileNotFoundError:
        return TestResult(test_name, False, error=f"Assembler not found: {assembler_path}")
    except Exception as e:
        return TestResult(test_name, False, error=str(e))


def run_syntax_tests(syntax, assembler_path=None, verbose=False):
    """Run all tests for a syntax module."""
    if syntax not in SYNTAX_CONFIG:
        print(f"Error: Unknown syntax module '{syntax}'")
        print(f"Available: {', '.join(SYNTAX_CONFIG.keys())}")
        return False

    config = SYNTAX_CONFIG[syntax]
    project_root = find_project_root()

    # Determine assembler path (resolve to absolute to avoid PATH conflicts)
    if assembler_path is None:
        assembler_path = (project_root / config['assembler']).resolve()
    else:
        assembler_path = Path(assembler_path).resolve()

    # Check assembler exists
    if not assembler_path.exists():
        print(f"Error: Assembler not found: {assembler_path}")
        print(f"Build it first with: make CPU=6502 SYNTAX={syntax}")
        return False

    # Find test files
    test_dir = project_root / config['test_dir']
    if not test_dir.exists():
        print(f"Error: Test directory not found: {test_dir}")
        return False

    test_files = sorted(glob.glob(str(test_dir / config['pattern'])))
    if not test_files:
        print(f"Warning: No test files found matching {config['pattern']} in {test_dir}")
        return True

    # Run tests
    print(f"=== {syntax.upper()} Syntax Tests ===")
    print(f"Assembler: {assembler_path}")
    print(f"Test directory: {test_dir}")
    print(f"Found {len(test_files)} test files")
    print()

    results = []
    with tempfile.TemporaryDirectory() as tmpdir:
        for test_file in test_files:
            result = run_test(str(assembler_path), test_file, tmpdir)
            results.append(result)

            if result.passed:
                if verbose:
                    print(f"  PASS: {result.name}")
            else:
                print(f"  FAIL: {result.name}")
                if result.error:
                    for line in result.error.split('\n')[:3]:
                        print(f"        {line}")

    # Summary
    passed = sum(1 for r in results if r.passed)
    failed = len(results) - passed

    print()
    print(f"{syntax.upper()}: {passed} passed, {failed} failed")

    return failed == 0


def main():
    parser = argparse.ArgumentParser(
        description='Run vasm syntax module tests',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s scmasm             Run SCMASM tests
  %(prog)s merlin             Run Merlin tests
  %(prog)s scmasm -v          Run SCMASM tests with verbose output
  %(prog)s merlin -a ./my_asm Run Merlin tests with custom assembler
  %(prog)s all                Run all syntax module tests
"""
    )
    parser.add_argument('syntax',
                        help='Syntax module to test (scmasm, merlin, oldstyle, or "all")')
    parser.add_argument('-a', '--assembler',
                        help='Path to assembler executable')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Show all test results, not just failures')

    args = parser.parse_args()

    if args.syntax == 'all':
        # Run all syntax modules
        all_passed = True
        for syntax in SYNTAX_CONFIG:
            test_dir = find_project_root() / SYNTAX_CONFIG[syntax]['test_dir']
            if test_dir.exists():
                if not run_syntax_tests(syntax, verbose=args.verbose):
                    all_passed = False
                print()

        print("=== Summary ===")
        if all_passed:
            print("ALL TESTS PASSED!")
            sys.exit(0)
        else:
            print("SOME TESTS FAILED")
            sys.exit(1)
    else:
        if run_syntax_tests(args.syntax, args.assembler, args.verbose):
            print("ALL TESTS PASSED!")
            sys.exit(0)
        else:
            print("SOME TESTS FAILED")
            sys.exit(1)


if __name__ == '__main__':
    main()
