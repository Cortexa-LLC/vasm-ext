#!/bin/bash
# Integrate new upstream vasm release into vasm-ext
# Run this after updating vasm repository with update_upstream.sh

set -e

VASM_EXT_DIR="/Users/bryanw/Projects/Vintage/tools/vasm-ext"
VASM_DIR="/Users/bryanw/Projects/Vintage/tools/vasm"

echo "=========================================="
echo "vasm-ext Upstream Integration"
echo "=========================================="
echo ""

cd "$VASM_EXT_DIR"

# Check git status
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Error: Uncommitted changes detected!"
    echo "Commit or stash changes before integrating."
    git status
    exit 1
fi

# Check we're on main
CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "main" ]]; then
    echo "Warning: Not on 'main' branch (currently on '$CURRENT_BRANCH')"
    read -p "Switch to main? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git checkout main
    else
        echo "Aborted."
        exit 1
    fi
fi

echo "Step 1: Fetching from upstream-repo..."
git fetch upstream-repo

echo ""
echo "Step 2: Checking for upstream changes..."
git checkout upstream-tracking

# Check if there are changes
if git diff --quiet upstream-repo/upstream; then
    echo "No changes in upstream - already up to date!"
    git checkout main
    exit 0
fi

echo ""
echo "Changes detected in upstream:"
git log --oneline upstream-tracking..upstream-repo/upstream
echo ""

read -p "Continue with integration? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted."
    git checkout main
    exit 0
fi

echo ""
echo "Step 3: Updating upstream-tracking branch..."
git reset --hard upstream-repo/upstream

# Prompt for new version tag
echo ""
read -p "Enter new upstream version (e.g., 2.0f): " NEW_VERSION
if [[ -n "$NEW_VERSION" ]]; then
    BASELINE_TAG="baseline-v${NEW_VERSION}"
    git tag -a "$BASELINE_TAG" -m "Baseline: Clean vasm $NEW_VERSION before extensions"
    echo "✓ Tagged $BASELINE_TAG"
fi

echo ""
echo "Step 4: Creating integration branch..."
git checkout main
INTEGRATE_BRANCH="integrate-upstream-v${NEW_VERSION:-next}"
git checkout -b "$INTEGRATE_BRANCH"

echo ""
echo "Step 5: Merging upstream changes..."
echo ""
echo "This may create conflicts in:"
echo "  - README.md (merge our Apple II section)"
echo "  - Makefile (if modified)"
echo "  - Other files we changed"
echo ""
echo "Our additions will be preserved:"
echo "  - syntax/scasm/"
echo "  - syntax/merlin/"
echo "  - CLAUDE.md"
echo "  - MAINTENANCE.md"
echo ""

if git merge upstream-tracking --no-edit; then
    echo ""
    echo "✓ Merge completed without conflicts!"
else
    echo ""
    echo "⚠ Conflicts detected!"
    echo ""
    echo "Resolve conflicts, then:"
    echo "  git add <resolved-files>"
    echo "  git commit"
    echo ""
    echo "After resolving, continue with:"
    echo "  git checkout main"
    echo "  git merge --no-ff $INTEGRATE_BRANCH"
    echo "  git tag -a v${NEW_VERSION}-ext1 -m 'vasm $NEW_VERSION with extensions'"
    echo "  git push origin main"
    echo "  git push --tags"
    exit 1
fi

echo ""
echo "Step 6: Testing integration..."
echo ""
echo "Build and test both modules:"
echo "  make CPU=6502 SYNTAX=scasm"
echo "  make CPU=6502 SYNTAX=merlin"
echo ""
read -p "Run tests now? (y/N) " -n 1 -r
echo

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Building SCASM module..."
    make CPU=6502 SYNTAX=scasm clean
    make CPU=6502 SYNTAX=scasm

    echo ""
    echo "Building Merlin module..."
    make CPU=6502 SYNTAX=merlin clean
    make CPU=6502 SYNTAX=merlin

    echo ""
    echo "✓ Both modules built successfully"
fi

echo ""
echo "Step 7: Finalizing integration..."
read -p "Merge to main? (y/N) " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Integration branch created: $INTEGRATE_BRANCH"
    echo "Review and merge manually when ready:"
    echo "  git checkout main"
    echo "  git merge --no-ff $INTEGRATE_BRANCH"
    exit 0
fi

echo ""
echo "Merging to main..."
git checkout main
git merge --no-ff "$INTEGRATE_BRANCH" -m "Integrate vasm $NEW_VERSION upstream changes

Merged upstream vasm $NEW_VERSION while preserving:
- SCASM syntax module (100% complete)
- Merlin syntax module (100% complete)
- Apple II documentation and test suites

Integration branch: $INTEGRATE_BRANCH"

# Tag the release
if [[ -n "$NEW_VERSION" ]]; then
    RELEASE_TAG="v${NEW_VERSION}-ext1"
    git tag -a "$RELEASE_TAG" -m "vasm $NEW_VERSION with SCASM and Merlin extensions"
    echo "✓ Tagged $RELEASE_TAG"
fi

echo ""
echo "=========================================="
echo "Integration Complete!"
echo "=========================================="
echo ""
git log --oneline -5
echo ""

if [[ -n "$NEW_VERSION" ]]; then
    echo "Tags created:"
    echo "  $BASELINE_TAG"
    echo "  $RELEASE_TAG"
    echo ""
fi

echo "To push to GitHub:"
echo "  git push origin main"
echo "  git push origin upstream-tracking"
echo "  git push --tags"
echo ""
echo "Integration branch can be deleted:"
echo "  git branch -d $INTEGRATE_BRANCH"
echo ""
