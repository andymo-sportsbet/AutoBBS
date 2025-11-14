#!/bin/bash
# Build script for AsirikuyFrameworkAPI and UnitTests
# Automates the complete build process on macOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
CONFIG="debug64"
PROJECT="AsirikuyFrameworkAPI"
CLEAN=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean)
      CLEAN=true
      shift
      ;;
    -v|--verbose)
      VERBOSE=true
      shift
      ;;
    -p|--project)
      PROJECT="$2"
      shift 2
      ;;
    -config)
      CONFIG="$2"
      shift 2
      ;;
    -h|--help)
      echo "Usage: $0 [OPTIONS]"
      echo ""
      echo "Options:"
      echo "  -c, --clean       Clean build directories before building"
      echo "  -v, --verbose     Verbose output"
      echo "  -p, --project     Project to build (default: AsirikuyFrameworkAPI)"
      echo "                    Options: AsirikuyFrameworkAPI, UnitTests, TradingStrategies, CTesterFrameworkAPI, all"
      echo "  -config          Build configuration (default: debug64)"
      echo "                    Options: debug32, debug64, release32, release64"
      echo "  -h, --help        Show this help message"
      echo ""
      echo "Examples:"
      echo "  $0                                    # Build AsirikuyFrameworkAPI (debug64)"
      echo "  $0 -c                                 # Clean build"
      echo "  $0 -p UnitTests                       # Build UnitTests"
      echo "  $0 -p all -config release64            # Build all projects (release64)"
      exit 0
      ;;
    *)
      echo -e "${RED}Unknown option: $1${NC}"
      echo "Use -h or --help for usage information"
      exit 1
      ;;
  esac
done

# Check if we're on macOS
if [ "$(uname -s)" != "Darwin" ]; then
  echo -e "${YELLOW}Warning: This script is optimized for macOS. Building on other platforms may require adjustments.${NC}"
fi

# Set BOOST_ROOT if not already set
if [ -z "$BOOST_ROOT" ]; then
  if [ -d "/Users/andym/homebrew/opt/boost" ]; then
    export BOOST_ROOT=/Users/andym/homebrew/opt/boost
    echo -e "${GREEN}Set BOOST_ROOT=$BOOST_ROOT${NC}"
  elif command -v brew &> /dev/null; then
    export BOOST_ROOT=$(brew --prefix boost)
    echo -e "${GREEN}Set BOOST_ROOT=$BOOST_ROOT${NC}"
  else
    echo -e "${RED}Error: BOOST_ROOT not set and brew not found.${NC}"
    echo "Please set BOOST_ROOT environment variable:"
    echo "  export BOOST_ROOT=/path/to/boost"
    exit 1
  fi
fi

# Check if premake4 Docker image exists
if ! docker images | grep -q "autobbs-premake4"; then
  echo -e "${YELLOW}Warning: Docker image 'autobbs-premake4' not found.${NC}"
  echo "You may need to build it first or use a different premake4 setup."
fi

echo -e "${GREEN}=== Starting Build Process ===${NC}"
echo "Project: $PROJECT"
echo "Configuration: $CONFIG"
echo "BOOST_ROOT: $BOOST_ROOT"
echo ""

# Step 1: Clean if requested
if [ "$CLEAN" = true ]; then
  echo -e "${YELLOW}Cleaning build directories...${NC}"
  rm -rf build/gmake bin/gmake tmp/gmake
  echo -e "${GREEN}✓ Clean complete${NC}"
  echo ""
fi

# Step 2: Generate Makefiles
echo -e "${YELLOW}Step 1: Generating Makefiles...${NC}"
if [ "$VERBOSE" = true ]; then
  docker run --rm -v "$(pwd):/workspace" -w /workspace -e BOOST_ROOT="$BOOST_ROOT" autobbs-premake4:latest premake4 gmake
else
  docker run --rm -v "$(pwd):/workspace" -w /workspace -e BOOST_ROOT="$BOOST_ROOT" autobbs-premake4:latest premake4 gmake 2>&1 | tail -3
fi
echo -e "${GREEN}✓ Makefiles generated${NC}"
echo ""

# Step 3: Fix Makefiles (remove -std=c++11 from CFLAGS, remove -lrt on macOS, rename .so to .dylib)
echo -e "${YELLOW}Step 2: Fixing Makefiles...${NC}"
./fix_makefiles.sh
echo -e "${GREEN}✓ Makefiles fixed${NC}"
echo ""

# Step 4: Build
echo -e "${YELLOW}Step 3: Building...${NC}"
cd build/gmake
export BOOST_ROOT="$BOOST_ROOT"

if [ "$PROJECT" = "all" ]; then
  # Build all projects
  if [ "$VERBOSE" = true ]; then
    make -j4 config="$CONFIG"
  else
    make -j4 config="$CONFIG" 2>&1 | grep -E "Building|Linking|Error|error" | tail -20
  fi
else
  # Build specific project
  if [ "$VERBOSE" = true ]; then
    make "$PROJECT" config="$CONFIG" -j4
  else
    make "$PROJECT" config="$CONFIG" -j4 2>&1 | tail -15
  fi
fi

cd ../..

# Step 4.5: Rename .so to .dylib on macOS (in case build created new .so files)
if [ "$(uname -s)" = "Darwin" ]; then
  echo -e "${YELLOW}Renaming .so files to .dylib on macOS...${NC}"
  find bin/gmake -name "*.so" -type f 2>/dev/null | while read sofile; do
    dylibfile="${sofile%.so}.dylib"
    if [ -f "$sofile" ]; then
      if [ -f "$dylibfile" ]; then
        # If .dylib already exists, remove the .so file
        rm -f "$sofile" 2>/dev/null || true
      else
        # Rename .so to .dylib
        mv "$sofile" "$dylibfile" 2>/dev/null || true
      fi
    fi
  done
  echo -e "${GREEN}✓ Rename complete${NC}"
  echo ""
fi

# Step 5: Verify build output
echo ""
echo -e "${YELLOW}Step 4: Verifying build output...${NC}"

if [ "$PROJECT" = "AsirikuyFrameworkAPI" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.dylib" ] || [ -f "bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.so" ]; then
    echo -e "${GREEN}✓ AsirikuyFrameworkAPI built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/libAsirikuyFrameworkAPI.* 2>/dev/null | head -1
  else
    echo -e "${RED}✗ AsirikuyFrameworkAPI build failed - output not found${NC}"
  fi
fi

if [ "$PROJECT" = "UnitTests" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/UnitTests" ]; then
    echo -e "${GREEN}✓ UnitTests built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/UnitTests
  else
    echo -e "${YELLOW}⚠ UnitTests not found (may not be built)${NC}"
  fi
fi

if [ "$PROJECT" = "TradingStrategies" ] || [ "$PROJECT" = "all" ]; then
  if [ -f "bin/gmake/x64/Debug/lib/libtrading_strategies.dylib" ] || [ -f "bin/gmake/x64/Debug/lib/libtrading_strategies.so" ]; then
    echo -e "${GREEN}✓ TradingStrategies built successfully${NC}"
    ls -lh bin/gmake/x64/Debug/lib/libtrading_strategies.* 2>/dev/null | head -1
  else
    echo -e "${YELLOW}⚠ TradingStrategies not found (may not be built)${NC}"
  fi
fi

if [ "$PROJECT" = "CTesterFrameworkAPI" ] || [ "$PROJECT" = "all" ]; then
  # CTesterFrameworkAPI output is in bin/gmake/x64/Debug/lib/ directory
  CTESTER_DYLIB=$(find bin/gmake -name "*CTesterFrameworkAPI.dylib" -o -name "*CTesterFrameworkAPI.so" 2>/dev/null | head -1)
  if [ -n "$CTESTER_DYLIB" ]; then
    echo -e "${GREEN}✓ CTesterFrameworkAPI built successfully${NC}"
    ls -lh "$CTESTER_DYLIB"
  else
    echo -e "${YELLOW}⚠ CTesterFrameworkAPI not found (may not be built)${NC}"
  fi
fi

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"

