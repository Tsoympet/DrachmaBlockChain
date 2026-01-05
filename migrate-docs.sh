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
mkdir -p docs/getting-started
mkdir -p docs/user-guides
mkdir -p docs/developer-guides
mkdir -p docs/operators
mkdir -p docs/technical-specs
mkdir -p docs/reference
mkdir -p docs/security
mkdir -p docs/assets
echo "✅ Directories created"
echo ""

# Move existing files
echo "📄 Moving existing files..."
[ -f "docs/staking-guide.md" ] && mv docs/staking-guide.md docs/user-guides/ && echo "  ✓ Moved staking-guide.md"
[ -f "docs/mining-guide.md" ] && mv docs/mining-guide.md docs/user-guides/ && echo "  ✓ Moved mining-guide.md"
[ -f "docs/consensus.md" ] && mv docs/consensus.md docs/technical-specs/ && echo "  ✓ Moved consensus.md"
[ -f "docs/protocol.md" ] && mv docs/protocol.md docs/technical-specs/ && echo "  ✓ Moved protocol.md"
[ -f "ECONOMICS.md" ] && mv ECONOMICS.md docs/reference/economics.md && echo "  ✓ Moved ECONOMICS.md"
[ -f "ARCHITECTURE.md" ] && mv ARCHITECTURE.md docs/reference/architecture.md && echo "  ✓ Moved ARCHITECTURE.md"
[ -f "ROADMAP.md" ] && mv ROADMAP.md docs/reference/roadmap.md && echo "  ✓ Moved ROADMAP.md"
[ -f "SECURITY.md" ] && mv SECURITY.md docs/security/security-overview.md && echo "  ✓ Moved SECURITY.md"
[ -f "sidechain/sidechain_spec.md" ] && cp sidechain/sidechain_spec.md docs/technical-specs/sidechain-spec.md && echo "  ✓ Copied sidechain_spec.md"
echo ""

# Update links in markdown files
echo "🔗 Updating internal links..."
find docs -name "*.md" -type f 2>/dev/null | while read file; do
    sed -i \
        -e 's|](docs/staking-guide\.md)|](user-guides/staking-guide.md)|g' \
        -e 's|](docs/mining-guide\.md)|](user-guides/mining-guide.md)|g' \
        -e 's|](docs/consensus\.md)|](technical-specs/consensus.md)|g' \
        -e 's|](docs/protocol\.md)|](technical-specs/protocol.md)|g' \
        -e 's|](ECONOMICS\.md)|](reference/economics.md)|g' \
        -e 's|](ARCHITECTURE\.md)|](reference/architecture.md)|g' \
        -e 's|](ROADMAP\.md)|](reference/roadmap.md)|g' \
        -e 's|](SECURITY\.md)|](security/security-overview.md)|g' \
        -e 's|sidechain/sidechain_spec\.md|technical-specs/sidechain-spec.md|g' \
        "$file"
done

[ -f "README.md" ] && sed -i \
    -e 's|](ECONOMICS\.md)|](docs/reference/economics.md)|g' \
    -e 's|](ARCHITECTURE\.md)|](docs/reference/architecture.md)|g' \
    -e 's|](ROADMAP\.md)|](docs/reference/roadmap.md)|g' \
    -e 's|](SECURITY\.md)|](docs/security/security-overview.md)|g' \
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
