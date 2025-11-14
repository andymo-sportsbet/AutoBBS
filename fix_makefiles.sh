#!/bin/bash
# Fix generated Makefiles:
# 1. Remove -std=c++11 from CFLAGS (only for C++ files)
# 2. Remove -lrt from LIBS on macOS (rt is Linux-only)

set -e

echo "Fixing Makefiles..."

# Detect OS
OS=$(uname -s)

# Find all Makefiles in build/gmake
find build/gmake -name "*.make" -type f | while read makefile; do
  modified=false
  
  # Fix 1: Remove -std=c++11 from CFLAGS but keep it in CXXFLAGS
  if grep -q "CFLAGS.*-std=c++11" "$makefile"; then
    echo "  Fixing CFLAGS in: $makefile"
    # Use sed to remove -std=c++11 from CFLAGS lines
    sed -i.bak 's/CFLAGS[^=]*=\([^;]*\)-std=c++11\([^;]*\)/CFLAGS\1=\2/g' "$makefile" 2>/dev/null || \
    sed -i '' 's/CFLAGS[^=]*=\([^;]*\)-std=c++11\([^;]*\)/CFLAGS\1=\2/g' "$makefile" 2>/dev/null || true
    
    # Ensure -std=c++11 is in CXXFLAGS if not already
    if ! grep -q "CXXFLAGS.*-std=c++11" "$makefile"; then
      # Add -std=c++11 to CXXFLAGS
      sed -i.bak 's/\(CXXFLAGS[^=]*=\)/\1 -std=c++11 /g' "$makefile" 2>/dev/null || \
      sed -i '' 's/\(CXXFLAGS[^=]*=\)/\1 -std=c++11 /g' "$makefile" 2>/dev/null || true
    fi
    modified=true
  fi
  
  # Fix 1a: For C++ projects that need C++11 but don't have it in CXXFLAGS
  # Check if this is a C++ project (has .cpp files) and needs C++11
  if grep -q "\.cpp" "$makefile" && ! grep -q "CXXFLAGS.*-std=c++11" "$makefile"; then
    # Check if premake4.lua specified buildoptions{"-std=c++11"} (AsirikuyEasyTrade, AsirikuyCommon)
    if grep -q "AsirikuyEasyTrade\|AsirikuyCommon" "$makefile"; then
      echo "  Adding -std=c++11 to CXXFLAGS in: $makefile"
      # Add -std=c++11 to CXXFLAGS after the = sign
      sed -i.bak 's/\(CXXFLAGS[^=]*=\)\([^;]*\)/\1 -std=c++11 \2/g' "$makefile" 2>/dev/null || \
      sed -i '' 's/\(CXXFLAGS[^=]*=\)\([^;]*\)/\1 -std=c++11 \2/g' "$makefile" 2>/dev/null || true
      modified=true
    fi
  fi
  
  # Fix 2: Remove -lrt from LIBS on macOS (rt is Linux-only)
  if [ "$OS" = "Darwin" ] && grep -q "LIBS.*-lrt" "$makefile"; then
    echo "  Removing -lrt from LIBS in: $makefile (macOS doesn't have rt library)"
    # Try multiple patterns to catch -lrt in different positions
    sed -i.bak 's/ -lrt / /g' "$makefile" 2>/dev/null || \
    sed -i '' 's/ -lrt / /g' "$makefile" 2>/dev/null || true
    sed -i.bak 's/-lrt / /g' "$makefile" 2>/dev/null || \
    sed -i '' 's/-lrt / /g' "$makefile" 2>/dev/null || true
    sed -i.bak 's/ -lrt$/ /g' "$makefile" 2>/dev/null || \
    sed -i '' 's/ -lrt$/ /g' "$makefile" 2>/dev/null || true
    sed -i.bak 's/-lrt$/ /g' "$makefile" 2>/dev/null || \
    sed -i '' 's/-lrt$/ /g' "$makefile" 2>/dev/null || true
    modified=true
  fi
  
  # Fix 3: Fix CFLAGS syntax errors (missing += operator and trailing =)
  if grep -q "^  CFLAGS \$(CPPFLAGS)" "$makefile"; then
    echo "  Fixing CFLAGS syntax in: $makefile"
    # Fix missing += operator
    sed -i.bak 's/^  CFLAGS $(CPPFLAGS)/  CFLAGS += $(CPPFLAGS)/g' "$makefile" 2>/dev/null || \
    sed -i '' 's/^  CFLAGS $(CPPFLAGS)/  CFLAGS += $(CPPFLAGS)/g' "$makefile" 2>/dev/null || true
    # Remove trailing = character
    sed -i.bak 's/\(CFLAGS +=.*\) =$/\1/g' "$makefile" 2>/dev/null || \
    sed -i '' 's/\(CFLAGS +=.*\) =$/\1/g' "$makefile" 2>/dev/null || true
    modified=true
  fi
  
  # Fix 3a: Remove trailing = from CXXFLAGS as well
  if grep -q "CXXFLAGS.*=$" "$makefile"; then
    echo "  Fixing CXXFLAGS trailing = in: $makefile"
    sed -i.bak 's/\(CXXFLAGS.*\) =$/\1/g' "$makefile" 2>/dev/null || \
    sed -i '' 's/\(CXXFLAGS.*\) =$/\1/g' "$makefile" 2>/dev/null || true
    modified=true
  fi
  
  # Clean up backup files if created
  [ -f "${makefile}.bak" ] && rm -f "${makefile}.bak" 2>/dev/null || true
done

# Fix 3: Rename .so to .dylib on macOS for clarity
if [ "$OS" = "Darwin" ]; then
  echo "Renaming .so files to .dylib on macOS..."
  find bin/gmake -name "*.so" -type f 2>/dev/null | while read sofile; do
    dylibfile="${sofile%.so}.dylib"
    if [ -f "$sofile" ]; then
      if [ -f "$dylibfile" ]; then
        # If .dylib already exists, remove the .so file
        echo "  Removing duplicate .so file: $sofile (dylib exists)"
        rm -f "$sofile" 2>/dev/null || true
      else
        # Rename .so to .dylib
        echo "  Renaming: $sofile → $dylibfile"
        mv "$sofile" "$dylibfile" 2>/dev/null || true
      fi
    fi
  done
fi

echo "Makefile fixes complete."
