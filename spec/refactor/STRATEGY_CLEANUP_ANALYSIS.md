# Strategy Cleanup Analysis

## Overview

This document analyzes which strategies are actually used and identifies unused strategies that can be removed before refactoring.

**Analysis Date**: 2024  
**Purpose**: Clean up codebase before C++ refactoring

---

## Strategy Usage Analysis

### Strategies with MQL Expert Advisors (ACTIVE - 22 strategies)

These strategies have corresponding `.mq4` or `.mq5` Expert Advisor files and are actively used:

1. **WATUKUSHAY_FE_BB** (ID: 0) ✅
   - Files: `WatukushayFE_BB.mq4`, `WatukushayFE_BB.mq5`
   - Implementation: `runWatukushayFE_BB()`

2. **WATUKUSHAY_FE_CCI** (ID: 1) ✅
   - Files: `WatukushayFE_CCI.mq4`
   - Implementation: `runWatukushayFE_CCI()`

3. **ATIPAQ** (ID: 2) ✅
   - Files: `Atipaq.mq4`
   - Implementation: `runAtipaq()`

4. **AYOTL** (ID: 3) ✅
   - Files: `Ayotl.mq4`
   - Implementation: `runAyotl()`

5. **COATL** (ID: 4) ✅
   - Files: `Coatl.mq4`
   - Implementation: `runCoatl()`

6. **COMITL_BB** (ID: 5) ✅
   - Files: `Comitl_BB.mq4`
   - Implementation: `runComitl_BB()`

7. **COMITL_KC** (ID: 6) ✅
   - Files: `Comitl_KC.mq4`
   - Implementation: `runComitl_KC()`

8. **COMITL_PA** (ID: 7) ✅
   - Files: `Comitl_PA.mq4`
   - Implementation: `runComitl_PA()`

9. **GODS_GIFT_ATR** (ID: 8) ✅
   - Files: `GodsGiftATR.mq4`
   - Implementation: `runGodsGiftATR()`

10. **QALLARYI** (ID: 9) ✅
    - Files: `Qallaryi.mq4`
    - Implementation: `runQallaryi()`

11. **QUIMICHI** (ID: 10) ✅
    - Files: `Quimichi.mq4`
    - Implementation: `runQuimichi()`

12. **SAPAQ** (ID: 11) ✅
    - Files: `Sapaq.mq4`
    - Implementation: `runSapaq()`

13. **ASIRIKUY_BRAIN** (ID: 12) ✅
    - Files: `AsirikuyBrain.mq4`
    - Implementation: `runAsirikuyBrain()`

14. **TEYACANANI** (ID: 13) ✅
    - Files: `Teyacanani.mq4`, `Teyacanani.mq5`
    - Implementation: `runTeyacanani()`

15. **WATUKUSHAY_FE_RSI** (ID: 14) ✅
    - Files: `WatukushayFE_RSI.mq4`, `WatukushayFE_RSI.mq5`
    - Implementation: `runWatukushayFE_RSI()`

16. **RUPHAY** (ID: 15) ✅
    - Files: `Ruphay.mq4`
    - Implementation: `runRuphay()`

17. **TEST_EA** (ID: 16) ✅
    - Files: `TestEA.mq4`
    - Implementation: `runTestEA()`
    - **Note**: Test strategy, but has MQL file

18. **EURCHF_GRID** (ID: 17) ✅
    - Files: `EURCHF_grid.mq4`
    - Implementation: `runEURCHF_grid()`

19. **KANTU** (ID: 18) ✅
    - Files: `Kantu.mq4`
    - Implementation: `runKantu()`

20. **RECORD_BARS** (ID: 19) ✅
    - Files: `RecordBars.mq4`
    - Implementation: `runRecordBars()`

21. **MUNAY** (ID: 20) ✅
    - Files: `Munay.mq4`
    - Implementation: `runMunay()`

---

### Strategies Used via AutoBBS Dispatcher (ACTIVE - 11 strategies)

These strategies are called through `AutoBBS.c` dispatcher (strategy_mode parameter):

22. **AUTOBBS** (ID: 29) ✅
    - Dispatcher: `runAutoBBS()`
    - Routes to various `workoutExecutionTrend_*` functions based on `AUTOBBS_TREND_MODE`
    - **Note**: This is the main dispatcher, actively used

23. **AUTOBBSWEEKLY** (ID: 30) ✅
    - Uses same `runAutoBBS()` dispatcher
    - Sets `strategy_mode = 1` (weekly)

24. **KEYK** (ID: 28) ✅
    - Implementation: `runKeyK()`
    - Called via AutoBBS dispatcher

25. **BBS** (ID: 25) ✅
    - Implementation: `runBBS()`
    - Called via AutoBBS dispatcher

26. **TRENDLIMIT** (ID: 31) ✅
    - Implementation: `runTrendLimit()`
    - Called via AutoBBS dispatcher

27. **BUDAN** (ID: 32) ✅
    - Implementation: `runBuDan()`
    - Called via AutoBBS dispatcher

28. **KELPIE** (ID: 24) ✅
    - Implementation: `runKelpie()`
    - Called via AutoBBS dispatcher

29. **TAKEOVER** (ID: 26) ✅
    - Implementation: `runTakeOver()`
    - Called via AutoBBS dispatcher

30. **SCREENING** (ID: 27) ✅
    - Implementation: `runScreening()`
    - Called via AutoBBS dispatcher

31. **KANTU_ML** (ID: 22) ✅
    - Implementation: `runKantuML()`
    - Called via AutoBBS dispatcher

32. **RENKO_TEST** (ID: 21) ✅
    - Implementation: `runRenkoTest()`
    - **Note**: Test strategy, but may be used for testing

---

### UNUSED STRATEGIES (Candidates for Removal)

#### 1. KANTU_RL (ID: 23) ❌ **REMOVE**
- **Status**: Defined in enum but NO implementation found
- **Evidence**: 
  - Enum has `KANTU_RL = 23`
  - No `runKantuRL()` function exists
  - No case handler in switch statement
  - No MQL file
- **Action**: Remove from enum

#### 2. Duplicate Kelpie Files ❌ **REMOVE**
- **Kelpie_back.c**: Backup/old version
- **Kelpie2.c**: Alternative version
- **Current**: `Kelpie.c` is the active version
- **Evidence**: `runKelpie()` in `Kelpie.c` calls `runKelpie_BBS_1H_NoDailyTrend()`
- **Action**: Remove `Kelpie_back.c` and `Kelpie2.c`

---

## Summary

### Active Strategies: 32
- 22 with MQL Expert Advisors
- 11 used via AutoBBS dispatcher (some overlap)

### Unused/Dead Code: 3 items
1. **KANTU_RL** enum value (no implementation)
2. **Kelpie_back.c** (backup file)
3. **Kelpie2.c** (alternative version)

---

## Cleanup Actions

### Action 1: Remove KANTU_RL from enum
**File**: `dev/TradingStrategies/src/AsirikuyStrategies.c`
- Remove `KANTU_RL = 23,` from enum
- No case handler exists, so no other changes needed

### Action 2: Remove duplicate Kelpie files
**Files to delete**:
- `dev/TradingStrategies/src/strategies/Kelpie_back.c`
- `dev/TradingStrategies/src/strategies/Kelpie2.c`

**Note**: Keep `Kelpie.c` as it's the active implementation

### Action 3: Verify no references
Before deletion, verify:
- [ ] No includes of `Kelpie_back.h` or `Kelpie2.h`
- [ ] No function calls to `runKelpie_back()` or `runKelpie2()`
- [ ] No references in build files

---

## Files to Remove

1. `dev/TradingStrategies/src/strategies/Kelpie_back.c`
2. `dev/TradingStrategies/src/strategies/Kelpie2.c`

## Code Changes

1. **AsirikuyStrategies.c** (line 106):
   - Remove: `KANTU_RL = 23,`

---

## Verification Checklist

Before removal:
- [ ] Search for any references to `KANTU_RL`
- [ ] Search for any references to `Kelpie_back`
- [ ] Search for any references to `Kelpie2`
- [ ] Check build system (premake4.lua) for file references
- [ ] Verify no header files reference these

After removal:
- [ ] Code compiles successfully
- [ ] No broken references
- [ ] Build system updated

---

## Impact Assessment

### Risk Level: LOW
- KANTU_RL: Never implemented, no risk
- Kelpie_back/Kelpie2: Backup files, not referenced

### Benefits
- Cleaner codebase
- Reduced confusion
- Easier refactoring
- Smaller codebase to migrate

---

**Document Version**: 1.0  
**Status**: Ready for Implementation

