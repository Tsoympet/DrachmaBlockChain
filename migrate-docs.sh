#!/bin/bash
set -e

echo "🚀 Starting DRACHMA Documentation Migration..."
echo ""

# Create backup
echo "📦 Creating backup..."
BACKUP_DIR="docs-backup-$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
[ -d "docs" ] && cp -r docs "$BACKUP_DIR/" && echo "✅ Backup:  $BACKUP_DIR"
echo ""

# Create new structure
echo "📁 Creating directory structure..."
mkdir -p doc/getting-started
mkdir -p doc/user-guides
mkdir -p doc/developer-guides
mkdir -p doc/operators
mkdir -p doc/technical-specs
mkdir -p doc/reference
mkdir -p doc/security
mkdir -p doc/assets
echo "✅ Directories created"
echo ""

# Move existing files
echo "📄 Moving existing files..."
[ -f "doc/staking-guide.md" ] && mv doc/staking-guide.md doc/user-guides/ && echo "  ✓ Moved staking-guide.md"
[ -f "doc/mining-guide.md" ] && mv doc/mining-guide.md doc/user-guides/ && echo "  ✓ Moved mining-guide.md"
[ -f "doc/consensus.md" ] && mv doc/consensus.md doc/technical-specs/ && echo "  ✓ Moved consensus.md"
[ -f "doc/protocol.md" ] && mv doc/protocol.md doc/technical-specs/ && echo "  ✓ Moved protocol.md"
[ -f "ECONOMICS.md" ] && mv ECONOMICS.md doc/reference/economics.md && echo "  ✓ Moved ECONOMICS.md"
[ -f "ARCHITECTURE.md" ] && mv ARCHITECTURE.md doc/reference/architecture.md && echo "  ✓ Moved ARCHITECTURE.md"
[ -f "ROADMAP.md" ] && mv ROADMAP.md doc/reference/roadmap.md && echo "  ✓ Moved ROADMAP.md"
[ -f "SECURITY.md" ] && mv SECURITY.md doc/security/security-overview.md && echo "  ✓ Moved SECURITY.md"
[ -f "sidechain/sidechain_spec.md" ] && cp sidechain/sidechain_spec.md doc/technical-specs/sidechain-spec.md && echo "  ✓ Copied sidechain_spec.md"
echo ""

# Update links in markdown files
echo "🔗 Updating internal links..."
find docs -name "*.md" -type f 2>/dev/null | while read file; do
    sed -i \
        -e 's|](doc/staking-guide\.md)|](user-guides/staking-guide.md)|g' \
        -e 's|](doc/mining-guide\.md)|](user-guides/mining-guide.md)|g' \
        -e 's|](doc/consensus\.md)|](technical-specs/consensus.md)|g' \
        -e 's|](doc/protocol\.md)|](technical-specs/protocol.md)|g' \
        -e 's|](ECONOMICS\.md)|](reference/economics.md)|g' \
        -e 's|](ARCHITECTURE\.md)|](reference/architecture.md)|g' \
        -e 's|](ROADMAP\.md)|](reference/roadmap.md)|g' \
        -e 's|](SECURITY\.md)|](security/security-overview.md)|g' \
        -e 's|sidechain/sidechain_spec\.md|technical-specs/sidechain-spec.md|g' \
        "$file"
done

[ -f "README.md" ] && sed -i \
    -e 's|](ECONOMICS\.md)|](doc/reference/economics.md)|g' \
    -e 's|](ARCHITECTURE\.md)|](doc/reference/architecture.md)|g' \
    -e 's|](ROADMAP\.md)|](doc/reference/roadmap.md)|g' \
    -e 's|](SECURITY\.md)|](doc/security/security-overview.md)|g' \
    "README.md" && echo "  ✓ Updated README.md"

echo "✅ Links updated"
echo ""

# Show structure
echo "📊 New structure:"
tree docs -L 2 2>/dev/null || find docs -type d | sort
echo ""

echo "✅ Migration complete!"
echo ""
echo "Files ready to commit.  Check status with:  git status"
