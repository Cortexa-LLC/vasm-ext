# vasm Maintenance Strategy

## Problem Statement

The upstream vasm project (http://sun.hasenbraten.de/vasm/) is not maintained in a public git repository. Changes are released as periodic source drops (tarballs). We need to maintain our extensions (SCASM and Merlin syntax modules) while being able to integrate upstream updates.

## Two-Repository Strategy

### Repository 1: vasm (Tracking Repository)

**Purpose:** Track clean upstream vasm releases without modifications

**Location:** `/Users/bryanw/Projects/Vintage/tools/vasm`

**GitHub:** `git@github.com:Cortexa-LLC/vasm.git`

**Branch Structure:**
- `upstream` - Clean upstream source drops, tagged by version
- No modifications, no extensions
- Each release gets a tag: `v2.0e`, `v2.1`, etc.

**Workflow:**
```bash
# For each new upstream release
cd /Users/bryanw/Projects/Vintage/tools/vasm
wget http://sun.hasenbraten.de/vasm/release/vasm.tar.gz
tar xzf vasm.tar.gz
rm -rf ./*   # Clear old files
cp -r vasm/* .
rm -rf vasm vasm.tar.gz
git add -A
git commit -m "Import vasm 2.1 upstream source drop

Date: $(date)
Source: http://sun.hasenbraten.de/vasm/release/vasm.tar.gz
Changes: See upstream history file"

git tag -a v2.1 -m "vasm 2.1 official release"
git push origin upstream
git push --tags
```

### Repository 2: vasm-ext (Extensions Repository)

**Purpose:** Our fork with SCASM and Merlin extensions

**Location:** `/Users/bryanw/Projects/Vintage/tools/vasm-ext`

**GitHub:** `git@github.com:Cortexa-LLC/vasm-ext.git`

**Branch Structure:**
```
main (or master)
  - Production-ready code
  - Stable, tested
  - Based on specific upstream version

upstream-tracking
  - Receives upstream source drops
  - Merge point for new versions
  - No local modifications

feature/*
  - Development branches for new features
  - Merged to main when ready

release/*
  - Release branches for specific versions
```

## Detailed Workflow

### Initial Setup (One-Time)

```bash
# 1. In vasm-ext, create upstream tracking branch
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
git checkout -b upstream-tracking 7dfc296  # Clean upstream commit

# 2. Add vasm as a remote (local path)
git remote add upstream-repo /Users/bryanw/Projects/Vintage/tools/vasm
git fetch upstream-repo

# 3. Tag baseline
git tag -a baseline-v2.0e -m "Baseline: Clean vasm 2.0e before extensions"

# 4. Return to main
git checkout main
```

### Integrating New Upstream Release

When a new vasm version is released (e.g., 2.1):

```bash
# 1. Update vasm repository (upstream tracking)
cd /Users/bryanw/Projects/Vintage/tools/vasm
wget http://sun.hasenbraten.de/vasm/release/vasm.tar.gz
tar xzf vasm.tar.gz
rm -rf ./*
cp -r vasm/* .
rm -rf vasm vasm.tar.gz
git add -A
git commit -m "Import vasm 2.1 upstream source drop"
git tag -a v2.1 -m "vasm 2.1 official release"
git push origin upstream
git push --tags

# 2. Update vasm-ext upstream-tracking branch
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
git checkout upstream-tracking
git fetch upstream-repo
git reset --hard upstream-repo/upstream
git tag -a baseline-v2.1 -m "Baseline: Clean vasm 2.1 before extensions"

# 3. Create integration branch
git checkout main
git checkout -b integrate-upstream-v2.1

# 4. Merge upstream changes
git merge upstream-tracking -m "Integrate vasm 2.1 upstream changes"

# This will likely have conflicts in:
# - Makefile (if we modified it)
# - vasm.c (version strings)
# - Any files we modified

# 5. Resolve conflicts
# Keep our additions:
#   - syntax/scasm/
#   - syntax/merlin/
#   - tests/scasm/
#   - tests/merlin/
#   - CLAUDE.md
#   - MAINTENANCE.md (this file)
#
# Merge carefully:
#   - README.md (merge our Apple II section with upstream changes)
#   - Makefile (keep our modifications)
#   - Any core files we patched (e.g., symbol.c for -nocase bug)

# 6. Test thoroughly
make CPU=6502 SYNTAX=scasm clean
make CPU=6502 SYNTAX=scasm
./vasm6502_scasm tests/scasm/test_basic.asm

make CPU=6502 SYNTAX=merlin clean
make CPU=6502 SYNTAX=merlin
./vasm6502_merlin tests/merlin/test_basic.asm

# 7. Commit integration
git add -A
git commit -m "Complete integration of vasm 2.1

Merged upstream vasm 2.1 changes while preserving:
- SCASM syntax module (100% complete)
- Merlin syntax module (100% complete)
- Apple II test suites
- Documentation

Conflicts resolved:
- [List files with conflicts]

Changes in upstream:
- [Summarize upstream changes from history file]"

# 8. Merge to main
git checkout main
git merge --no-ff integrate-upstream-v2.1
git tag -a v2.1-ext1 -m "vasm 2.1 with SCASM and Merlin extensions"
```

### Alternative: Rebase Strategy

If you prefer a cleaner history:

```bash
# Instead of merge, use rebase
git checkout integrate-upstream-v2.1
git rebase upstream-tracking

# Resolve conflicts for each commit
# Advantage: Linear history
# Disadvantage: More complex conflict resolution
```

## File Organization

### Our Additions (Never in Upstream)

These files are always kept, never conflict:

```
syntax/scasm/           # SCASM syntax module
syntax/merlin/          # Merlin syntax module
tests/scasm/            # SCASM tests
tests/merlin/           # Merlin tests
CLAUDE.md              # Claude Code guidance
MAINTENANCE.md         # This file
```

### Modified Upstream Files (Conflict Potential)

These require careful merging:

```
README.md              # Added Apple II section
Makefile               # May add targets
symbol.c               # Bug fixes (e.g., -nocase)
```

**Strategy for these:**
1. Keep detailed documentation of WHY we changed them
2. Use `git diff baseline-v2.0e..main -- symbol.c` to see our changes
3. Apply our changes as patches to new upstream version
4. Consider upstreaming bug fixes to vasm authors

### Pure Upstream Files

These are replaced wholesale on update:

```
vasm.c
atom.c
expr.c
parse.c
cpus/*/
output_*.c
(most core files)
```

## Tracking Our Modifications

### Create Patch Files

For easier reapplication of our changes:

```bash
# Generate patches for our core modifications
git diff baseline-v2.0e..main -- symbol.c > patches/0001-fix-nocase-flag.patch
git diff baseline-v2.0e..main -- README.md > patches/0002-add-apple-ii-docs.patch

# On new upstream:
cd /path/to/new-version
patch -p1 < patches/0001-fix-nocase-flag.patch
```

### Maintain MODIFICATIONS.md

Document every change to upstream files:

```markdown
# Modifications to Upstream vasm

## Bug Fixes

### symbol.c (Line 139-151)
**Issue:** -nocase flag broken due to hash table mismatch
**Fix:** Use find_name_nc() when nocase flag is set
**Upstream Status:** Not submitted (consider submitting)
**Affected Versions:** 2.0e and earlier

## Enhancements

### README.md
**Change:** Added "New Apple II Syntax Modules" section
**Lines:** 247-306
**Upstream Status:** Extension-specific, not for upstream
```

## Version Tagging Strategy

### Upstream Tags (in vasm-upstream)
```
v2.0e       - vasm 2.0e official
v2.1        - vasm 2.1 official
```

### Baseline Tags (in vasm-ext)
```
baseline-v2.0e   - Clean 2.0e before our changes
baseline-v2.1    - Clean 2.1 before our changes
```

### Extension Tags (in vasm-ext)
```
v2.0e-ext1       - First release with our extensions on 2.0e
v2.0e-ext2       - Second release (bug fixes) on 2.0e
v2.1-ext1        - First release with our extensions on 2.1
```

## Testing Strategy for Integration

### Automated Test Suite

Create `test-integration.sh`:

```bash
#!/bin/bash
set -e

echo "Testing SCASM module..."
make CPU=6502 SYNTAX=scasm clean
make CPU=6502 SYNTAX=scasm
./vasm6502_scasm -Fbin -o /tmp/test1.bin tests/scasm/test_basic.asm
./vasm6502_scasm -Fbin -o /tmp/test2.bin tests/scasm/test_delimiters.asm

echo "Testing Merlin module..."
make CPU=6502 SYNTAX=merlin clean
make CPU=6502 SYNTAX=merlin
./vasm6502_merlin -Fbin -o /tmp/test3.bin tests/merlin/test_basic.asm
./vasm6502_merlin -Fbin -o /tmp/test4.bin tests/merlin/test_macros.asm

echo "Testing oldstyle (upstream baseline)..."
make CPU=6502 SYNTAX=oldstyle clean
make CPU=6502 SYNTAX=oldstyle
./vasm6502_oldstyle -Fbin -o /tmp/test5.bin tests/oldstyle/test01.asm

echo "All tests passed!"
```

Run after every integration:
```bash
./test-integration.sh
```

## Git Commands Reference

### View Our Changes Since Baseline
```bash
# See all files we've modified
git diff baseline-v2.0e..main --name-only

# See specific file changes
git diff baseline-v2.0e..main -- symbol.c

# See our commits since baseline
git log baseline-v2.0e..main --oneline

# See only our added files
git diff baseline-v2.0e..main --diff-filter=A --name-only
```

### Cherry-Pick Specific Fixes
```bash
# If upstream added a fix we need
git cherry-pick <commit-hash>
```

### Diff Against Upstream
```bash
# Compare our main to upstream tracking
git diff upstream-tracking..main

# Show only modified files (not added)
git diff upstream-tracking..main --diff-filter=M --name-only
```

## Communication with Upstream

### Submitting Bug Fixes

If we fix bugs that benefit all vasm users:

1. **Test thoroughly** with standard syntax modules
2. **Create minimal test case**
3. **Email authors** (check vasm.pdf for contact info)
4. **Provide patch** in unified diff format:
   ```bash
   diff -u symbol.c.orig symbol.c > nocase-fix.patch
   ```

### Requesting Features

For feature requests (e.g., better plugin hooks):
1. Show use case
2. Propose minimal API changes
3. Offer to help test

## Disaster Recovery

### Lost Work?

```bash
# Find all branches that touched our modules
git log --all --oneline -- syntax/scasm syntax/merlin

# Recover from reflog
git reflog
git checkout <commit-hash>
git checkout -b recovery-branch
```

### Corrupt Merge?

```bash
# Abort merge and start over
git merge --abort

# Or reset to pre-merge state
git reset --hard HEAD@{1}
```

### Start Over from Baseline

```bash
# Nuclear option: reapply our changes cleanly
git checkout upstream-tracking
git checkout -b fresh-integration
git checkout main -- syntax/scasm syntax/merlin tests/scasm tests/merlin
git checkout main -- CLAUDE.md MAINTENANCE.md
git commit -m "Reapply extensions to new baseline"
```

## Maintenance Checklist

### When New Upstream Released

- [ ] Download new source drop
- [ ] Import to vasm-upstream repository
- [ ] Tag new version in vasm-upstream
- [ ] Update upstream-tracking branch in vasm-ext
- [ ] Tag new baseline in vasm-ext
- [ ] Create integration branch
- [ ] Merge or rebase upstream changes
- [ ] Resolve conflicts (document in commit message)
- [ ] Run full test suite
- [ ] Test SCASM module
- [ ] Test Merlin module
- [ ] Test unmodified modules (oldstyle)
- [ ] Update MODIFICATIONS.md if needed
- [ ] Merge to main
- [ ] Tag extension release
- [ ] Update documentation if upstream changed APIs
- [ ] Consider submitting bug fixes upstream

### Quarterly Maintenance

- [ ] Check for new upstream releases
- [ ] Review our modifications for upstreaming opportunities
- [ ] Update documentation
- [ ] Run full test suite
- [ ] Archive old release tags

## Directory Structure

```
/Users/bryanw/Projects/Vintage/tools/
├── vasm-ext/              # Extensions repository
│   ├── .git/
│   │   ├── origin → git@github.com:Cortexa-LLC/vasm-ext.git
│   │   └── upstream-repo → ../vasm (local)
│   ├── syntax/
│   │   ├── scasm/        # Our addition
│   │   ├── merlin/       # Our addition
│   │   ├── oldstyle/     # Upstream
│   │   └── ...
│   ├── tests/
│   │   ├── scasm/        # Our addition
│   │   ├── merlin/       # Our addition
│   │   └── ...
│   ├── CLAUDE.md         # Our addition
│   └── MAINTENANCE.md    # This file
│
└── vasm/                  # Upstream tracking repository
    ├── .git/
    │   └── origin → git@github.com:Cortexa-LLC/vasm.git
    └── [clean upstream files only]
```

## Best Practices

1. **Never commit directly to upstream-tracking**
   - Always import from vasm-upstream repo

2. **Tag everything**
   - Baselines, integrations, releases

3. **Document modifications**
   - Why we changed it
   - Where in the code
   - Whether it should go upstream

4. **Test before merging**
   - Our modules
   - Upstream modules (regression check)

5. **Keep integration branches**
   - Don't delete until next integration
   - Useful for reference

6. **Communicate with upstream**
   - Bug fixes benefit everyone
   - Good open source citizenship

## Quick Reference Commands

```bash
# Start new integration
git checkout main
git fetch upstream-repo
git checkout upstream-tracking
git reset --hard upstream-repo/upstream
git tag baseline-v2.X
git checkout main
git checkout -b integrate-v2.X
git merge upstream-tracking

# View our modifications
git diff baseline-v2.0e..main --name-only
git diff upstream-tracking..main

# Test everything
make CPU=6502 SYNTAX=scasm && make CPU=6502 SYNTAX=merlin

# Complete integration
git checkout main
git merge --no-ff integrate-v2.X
git tag v2.X-ext1
```

## Notes

- Both repositories on same machine for fast local remote access
- vasm: Clean upstream only → git@github.com:Cortexa-LLC/vasm.git
- vasm-ext: Extensions → git@github.com:Cortexa-LLC/vasm-ext.git
- Use local path for upstream-repo remote (faster than GitHub)
- Back up both repositories
- Document every conflict resolution
- Maintain patch files for critical changes

## Future Enhancements

- Automated script to check for new upstream releases
- CI/CD pipeline for testing integrations
- Automated patch generation/application
- Docker container for reproducible builds
