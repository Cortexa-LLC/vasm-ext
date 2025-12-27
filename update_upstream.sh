#!/bin/bash
# Update vasm upstream repository with latest source drop
# Usage: ./update_vasm_upstream.sh [release|daily]

set -e

VASM_DIR="/Users/bryanw/Projects/Vintage/tools/vasm"
BUILD_TYPE="${1:-daily}"  # Default to daily if not specified

# URLs
RELEASE_URL="http://sun.hasenbraten.de/vasm/release/vasm.tar.gz"
DAILY_URL="http://sun.hasenbraten.de/vasm/daily/vasm.tar.gz"

echo "=========================================="
echo "vasm Upstream Update Script"
echo "=========================================="
echo ""

# Validate build type
if [[ "$BUILD_TYPE" != "release" && "$BUILD_TYPE" != "daily" ]]; then
    echo "Error: Build type must be 'release' or 'daily'"
    echo "Usage: $0 [release|daily]"
    exit 1
fi

# Select URL
if [[ "$BUILD_TYPE" == "release" ]]; then
    URL="$RELEASE_URL"
    echo "Source: Official release build"
else
    URL="$DAILY_URL"
    echo "Source: Daily development build"
fi

echo "URL: $URL"
echo ""

# Check we're in the right repository
cd "$VASM_DIR"
if [[ ! -d ".git" ]]; then
    echo "Error: $VASM_DIR is not a git repository!"
    exit 1
fi

# Check we're on upstream branch
CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "upstream" ]]; then
    echo "Warning: Not on 'upstream' branch (currently on '$CURRENT_BRANCH')"
    read -p "Switch to upstream branch? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git checkout upstream
    else
        echo "Aborted."
        exit 1
    fi
fi

# Check for uncommitted changes
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Error: Uncommitted changes detected!"
    echo "Commit or stash changes before updating upstream."
    git status
    exit 1
fi

echo "Step 1: Downloading vasm from $URL..."
wget -q --show-progress "$URL" -O vasm.tar.gz

echo ""
echo "Step 2: Extracting tarball..."
tar xzf vasm.tar.gz

echo ""
echo "Step 3: Checking version information..."
if [[ -f "vasm/history" ]]; then
    echo "Latest history entries:"
    head -20 vasm/history
else
    echo "No history file found"
fi

echo ""
echo "Step 4: Checking for -nocase fix in symbol.c..."
if grep -q "if (nocase)" vasm/symbol.c 2>/dev/null; then
    echo "✓ -nocase fix present in upstream"
else
    echo "✗ -nocase fix NOT found (or symbol.c missing)"
fi

echo ""
read -p "Continue with update? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted. Cleaning up..."
    rm -rf vasm vasm.tar.gz
    exit 0
fi

echo ""
echo "Step 5: Extracting tarball over repository..."
# Remove inspection directory first
rm -rf vasm

# Extract directly, overwriting existing files
# --strip-components=1 removes the 'vasm/' wrapper directory
tar xzf vasm.tar.gz --strip-components=1

# Clean up
rm -f vasm.tar.gz

echo ""
echo "Step 6: Checking for changes..."
git add -A

if git diff --cached --quiet; then
    echo "No changes detected - already up to date!"
    exit 0
fi

echo ""
echo "Changes detected:"
git diff --cached --stat
echo ""
echo "Files changed: $(git diff --cached --numstat | wc -l | tr -d ' ')"
echo ""

# Show sample of actual changes
echo "Sample changes (first 100 lines):"
git diff --cached | head -100
echo ""
echo "[... use 'git diff --cached' to see all changes ...]"
echo ""

read -p "Commit these changes? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted. Discarding changes..."
    git reset --hard HEAD
    git clean -fd
    exit 0
fi

echo ""
echo "Step 7: Committing changes..."
COMMIT_DATE=$(date)

# Get change summary
CHANGES_SUMMARY=$(git diff --cached --stat)

git commit -m "Update to latest vasm $BUILD_TYPE build

Date: $COMMIT_DATE
Source: $URL
Build type: $BUILD_TYPE

Changes:
$CHANGES_SUMMARY"

echo "✓ Committed"
git log --oneline -1

# Check if we should tag
if [[ "$BUILD_TYPE" == "release" ]]; then
    echo ""
    read -p "Tag this release? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "Enter version tag (e.g., v2.0f): " VERSION_TAG
        if [[ -n "$VERSION_TAG" ]]; then
            git tag -a "$VERSION_TAG" -m "vasm $VERSION_TAG official release"
            echo "✓ Tagged as $VERSION_TAG"
        fi
    fi
fi

echo ""
echo "Step 8: Pushing to GitHub..."
read -p "Push to origin? (y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    git push origin upstream

    # Push tags if any
    if git tag --points-at HEAD | grep -q .; then
        git push --tags
        echo "✓ Pushed tags"
    fi

    echo "✓ Pushed to GitHub"
else
    echo "Skipped push. You can push later with:"
    echo "  git push origin upstream"
    if git tag --points-at HEAD | grep -q .; then
        echo "  git push --tags"
    fi
fi

echo ""
echo "=========================================="
echo "Update Complete!"
echo "=========================================="
echo ""
git log --oneline -1 --stat
echo ""
if git tag --points-at HEAD | grep -q .; then
    echo "Tags: $(git tag --points-at HEAD | tr '\n' ' ')"
    echo ""
fi
echo "Next steps for vasm-ext:"
echo "  cd /Users/bryanw/Projects/Vintage/tools/vasm-ext"
echo "  git fetch upstream-repo"
echo "  git checkout upstream-tracking"
echo "  git reset --hard upstream-repo/upstream"
echo ""
