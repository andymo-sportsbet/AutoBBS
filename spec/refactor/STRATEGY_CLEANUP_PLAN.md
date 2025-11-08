# Strategy Cleanup Plan

## Summary

**Unused Strategies Identified:**
1. **KANTU_RL** (ID: 23) - Enum value with no implementation
2. **Kelpie_back.c** - Backup/old version of Kelpie
3. **Kelpie2.c** - Alternative version of Kelpie

**Active Strategy**: `Kelpie.c` is the current implementation

---

## Cleanup Actions

### Action 1: Remove KANTU_RL from enum

**File**: `dev/TradingStrategies/src/AsirikuyStrategies.c`

**Change**: Remove line 106
```c
// REMOVE THIS LINE:
KANTU_RL          = 23,
```

**Impact**: None - no implementation exists, no case handler

---

### Action 2: Delete Kelpie_back.c

**File**: `dev/TradingStrategies/src/strategies/Kelpie_back.c`

**Reason**: Backup/old version, not referenced anywhere

**Action**: Delete file

---

### Action 3: Delete Kelpie2.c

**File**: `dev/TradingStrategies/src/strategies/Kelpie2.c`

**Reason**: Alternative version, not referenced anywhere

**Action**: Delete file

---

## Verification

Before removal, verify:
- ✅ No includes of `Kelpie_back.h` or `Kelpie2.h` (none found)
- ✅ No function calls to `runKelpie_back()` or `runKelpie2()` (none found)
- ✅ No references to `KANTU_RL` except enum definition (confirmed)
- ✅ Build system uses wildcards, will auto-exclude deleted files

---

## Files to Delete

1. `dev/TradingStrategies/src/strategies/Kelpie_back.c`
2. `dev/TradingStrategies/src/strategies/Kelpie2.c`

## Code Changes

1. `dev/TradingStrategies/src/AsirikuyStrategies.c` (line 106):
   - Remove: `KANTU_RL = 23,`

---

## Post-Cleanup Verification

After cleanup:
- [ ] Code compiles successfully
- [ ] No broken references
- [ ] All tests pass
- [ ] Build system works

---

**Status**: Ready for Execution

