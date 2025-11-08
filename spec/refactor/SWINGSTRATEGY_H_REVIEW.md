# SwingStrategy.h Function Review

## Overview
This document reviews all functions declared in `dev/TradingStrategies/include/strategies/SwingStrategy.h` to identify any that should be removed.

**Review Date**: 2024  
**File**: `dev/TradingStrategies/include/strategies/SwingStrategy.h`  
**Total Functions Declared**: 20 (10 split*Orders pairs + 6 workoutExecutionTrend_* functions)

---

## Function Analysis

### ✅ ACTIVE - Keep These Functions

#### Order Splitting Functions (All Called from AutoBBS.c)

1. **`splitBuyOrders_Swing` / `splitSellOrders_Swing`** (Lines 20-21)
   - ✅ Called in AutoBBS.c (lines 51, 150)
   - ✅ Used in splitTradeMode case 13
   - **Status**: KEEP

2. **`splitBuyOrders_Daily_Swing` / `splitSellOrders_Daily_Swing`** (Lines 23-24)
   - ✅ Called in AutoBBS.c (lines 60, 159)
   - ⚠️ Implemented as `static` in StrategyComLib.c (lines 778, 812) - **POTENTIAL ISSUE**
   - ✅ Used in splitTradeMode case 16
   - **Status**: KEEP (but note static implementation issue)

3. **`splitBuyOrders_Daily_Swing_Fix` / `splitSellOrders_Daily_Swing_Fix`** (Lines 26-27)
   - ✅ Called in AutoBBS.c (lines 66, 165)
   - ✅ Used in splitTradeMode case 18
   - **Status**: KEEP

4. **`splitBuyOrders_Daily_GBPJPY_Swing` / `splitSellOrders_Daily_GBPJPY_Swing`** (Lines 29-30)
   - ✅ Called in AutoBBS.c (lines 63, 162)
   - ✅ Used in splitTradeMode case 17
   - **Status**: KEEP

5. **`splitBuyOrders_Daily_XAUUSD_Swing` / `splitSellOrders_Daily_XAUUSD_Swing`** (Lines 33-34)
   - ✅ Called in AutoBBS.c (lines 75, 174)
   - ✅ Used in splitTradeMode case 21
   - **Status**: KEEP

6. **`splitBuyOrders_MultiDays_Swing` / `splitSellOrders_MultiDays_Swing`** (Lines 37-38)
   - ✅ Called in AutoBBS.c (lines 78, 177)
   - ✅ Used in splitTradeMode case 22
   - **Status**: KEEP

7. **`splitBuyOrders_MultiDays_Swing_V2` / `splitSellOrders_MultiDays_Swing_V2`** (Lines 40-41)
   - ✅ Called in AutoBBS.c (lines 102, 201)
   - ✅ Used in splitTradeMode case 30
   - **Status**: KEEP

8. **`splitBuyOrders_ShortTerm_ATR_Hedge` / `splitSellOrders_ShortTerm_ATR_Hedge`** (Lines 43-44)
   - ✅ Called in AutoBBS.c (lines 57, 156)
   - ✅ Implemented in StrategyComLib.c (lines 910, 932)
   - ✅ Used in splitTradeMode case 15
   - **Status**: KEEP

9. **`splitBuyOrders_ShortTerm_Hedge` / `splitSellOrders_ShortTerm_Hedge`** (Lines 46-47)
   - ✅ Called in AutoBBS.c (lines 54, 153)
   - ✅ Implemented in StrategyComLib.c (lines 954, 976)
   - ✅ Used in splitTradeMode case 14
   - **Status**: KEEP

#### Strategy Execution Functions

10. **`workoutExecutionTrend_Hedge`** (Line 50)
    - ✅ Called from `workoutExecutionTrend_Auto_Hedge` in StrategyComLib.c (line 1411)
    - ✅ Implemented in StrategyComLib.c (line 1795)
    - **Status**: KEEP

11. **`workoutExecutionTrend_Auto_Hedge`** (Line 51)
    - ✅ Called in AutoBBS.c (case 14, line 254)
    - ✅ Called in StrategyComLib.c (case 14, line 1305)
    - ✅ Implemented in StrategyComLib.c (line 1396)
    - **Status**: KEEP

12. **`workoutExecutionTrend_XAUUSD_DayTrading`** (Line 55)
    - ✅ Called in AutoBBS.c (case 15, line 257)
    - ✅ Called in StrategyComLib.c (case 15, line 1308)
    - ✅ Implemented in StrategyComLib.c (line 2621)
    - **Status**: KEEP

13. **`workoutExecutionTrend_MultipleDay`** (Line 58)
    - ✅ Called in AutoBBS.c (case 21, line 275)
    - **Status**: KEEP

14. **`workoutExecutionTrend_GBPJPY_DayTrading_Ver2`** (Line 62)
    - ✅ Called in AutoBBS.c (case 16, line 260)
    - **Status**: KEEP

15. **`workoutExecutionTrend_Weekly_Swing_New`** (Line 65)
    - ✅ Called in AutoBBS.c (case 13, line 251)
    - **Status**: KEEP

16. **`workoutExecutionTrend_MACD_BEILI`** (Line 68)
    - ✅ Called in AutoBBS.c (case 33, line 312)
    - **Status**: KEEP

17. **`workoutExecutionTrend_MultipleDay_V2`** (Line 70)
    - ✅ Called in AutoBBS.c (case 22, line 278)
    - **Status**: KEEP

---

### ❌ POTENTIALLY DEAD CODE - Review These

18. **`splitBuyOrders_DayTrading` / `splitSellOrders_DayTrading`** (Lines 17-18)
    - ⚠️ Called in AutoBBS.c (lines 29, 132) - used in splitTradeMode case 5
    - ❌ **NO IMPLEMENTATION FOUND** - This is a problem!
    - **Status**: **INVESTIGATE** - Either missing implementation or dead code
    - **Recommendation**: Check if these should be implemented or if case 5 is unused

---

## Summary

### Functions to Keep: 17 pairs/functions
All functions except `splitBuyOrders_DayTrading` and `splitSellOrders_DayTrading` are actively used.

### Functions Requiring Investigation: 2
- `splitBuyOrders_DayTrading` - Declared and called, but no implementation found
- `splitSellOrders_DayTrading` - Declared and called, but no implementation found

### Potential Issues Found

1. **Missing Implementations**: 
   - `splitBuyOrders_DayTrading` and `splitSellOrders_DayTrading` are called but not implemented
   - This will cause linker errors unless they're implemented elsewhere or the calls are dead code

2. **Static Implementation Issue**:
   - `splitBuyOrders_Daily_Swing` and `splitSellOrders_Daily_Swing` are implemented as `static` in StrategyComLib.c
   - But they're declared in the header and called from AutoBBS.c
   - This suggests there might be non-static implementations elsewhere, or this is a build configuration issue

---

## Recommendations

1. **Investigate `splitBuyOrders_DayTrading` / `splitSellOrders_DayTrading`**:
   - Check if case 5 in AutoBBS.c is actually used in production
   - If not used, remove the calls and declarations
   - If used, implement the missing functions

2. **Verify `splitBuyOrders_Daily_Swing` / `splitSellOrders_Daily_Swing`**:
   - Check if there are non-static implementations in SwingStrategy.c
   - If not, the static implementations in StrategyComLib.c won't work when called from AutoBBS.c
   - This may need to be fixed or the functions moved

3. **All other functions are actively used and should be kept**

---

## Next Steps

1. Search for implementations of `splitBuyOrders_DayTrading` and `splitSellOrders_DayTrading` in all source files
2. Check if splitTradeMode case 5 is ever set in production code
3. Verify the build system links correctly for `splitBuyOrders_Daily_Swing` functions

