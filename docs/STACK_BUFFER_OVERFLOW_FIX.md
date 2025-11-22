# Stack Buffer Overflow Fix

## Problem
The application was crashing with `EXC_CRASH (SIGABRT)` due to **stack buffer overflow** in `workoutExecutionTrend_MACD_Daily`.

### Crash Details
- **Exception Type**: `EXC_CRASH (SIGABRT)`
- **Termination Reason**: `stack buffer overflow`
- **Crashed Thread**: Thread 3
- **Location**: `workoutExecutionTrend_MACD_Daily` -> `iMACDTrendBeiLi` -> `__stack_chk_fail`

## Root Cause

### Issue 1: `iMACDTrendBeiLi` Function
**Location**: `core/AsirikuyEasyTrade/src/EasyTrade.cpp:1681`

**Problem**: Large local arrays allocated on the stack:
```cpp
double fast[300] = {}, slow[300] = {}, preHist[300] = {};
```

**Stack Usage**: 3 × 300 × 8 bytes = **7,200 bytes (~7KB)** on the stack

### Issue 2: `iBBandStop` Function  
**Location**: `core/AsirikuyEasyTrade/src/EasyTrade.cpp:1815`

**Problem**: Even larger local arrays allocated on the stack:
```cpp
double upperBand[2000] = {}, middleBand[2000] = {}, lowerBand[2000] = {};
```

**Stack Usage**: 3 × 2000 × 8 bytes = **48,000 bytes (~48KB)** on the stack

## Why This Causes Crashes

1. **Stack Size Limits**: Default stack size on macOS is typically 8MB per thread
2. **Deep Call Stack**: When called from `workoutExecutionTrend_MACD_Daily` (which already has many local variables), the stack can overflow
3. **Multi-threading**: With OpenMP, multiple threads each have their own stack, but the issue is compounded when threads call these functions simultaneously
4. **Stack Canary**: Modern compilers add stack canaries that detect overflows and abort the process

## Solution

### Fix Applied
Moved large arrays from stack to heap allocation using `malloc()` and `free()`:

**Before**:
```cpp
double fast[300] = {}, slow[300] = {}, preHist[300] = {};
```

**After**:
```cpp
// CRITICAL FIX: Move large arrays from stack to heap to prevent stack overflow
double* fast = (double*)malloc(300 * sizeof(double));
double* slow = (double*)malloc(300 * sizeof(double));
double* preHist = (double*)malloc(300 * sizeof(double));

// Initialize arrays to zero
if (fast && slow && preHist) {
    memset(fast, 0, 300 * sizeof(double));
    memset(slow, 0, 300 * sizeof(double));
    memset(preHist, 0, 300 * sizeof(double));
} else {
    // Memory allocation failed - return error
    if (fast) free(fast);
    if (slow) free(slow);
    if (preHist) free(preHist);
    return -1; // Error code
}

// ... use arrays ...

// Free heap-allocated arrays before returning
free(fast);
free(slow);
free(preHist);
return 0;
```

### Changes Made

1. **`iMACDTrendBeiLi`** (`EasyTrade.cpp:1679-1808`):
   - Moved `fast[300]`, `slow[300]`, `preHist[300]` to heap
   - Added proper cleanup at all return points
   - Added error handling for allocation failures

2. **`iBBandStop`** (`EasyTrade.cpp:1811-1899`):
   - Moved `upperBand[2000]`, `middleBand[2000]`, `lowerBand[2000]` to heap
   - Added proper cleanup at all return points
   - Added error handling for allocation failures

3. **Includes Added**:
   - Added `#include <stdlib.h>` for `malloc()` and `free()`
   - Added `#include <string.h>` for `memset()`

## Impact

### Before Fix
- **Stack Usage**: ~55KB+ per function call (7KB + 48KB)
- **Risk**: High probability of stack overflow, especially in multi-threaded scenarios
- **Symptoms**: Intermittent crashes with `stack buffer overflow`

### After Fix
- **Stack Usage**: Minimal (only pointers and small variables)
- **Heap Usage**: Arrays allocated on heap (automatically managed)
- **Risk**: Eliminated stack overflow risk
- **Performance**: Negligible impact (heap allocation is fast for these sizes)

## Testing

After the fix:
1. Run stress tests to verify no more stack overflow crashes
2. Monitor memory usage to ensure proper cleanup
3. Verify performance is not significantly impacted

## Related Issues

This fix addresses:
- **Early exit issue**: Stack overflow crashes could cause the test to exit early
- **Intermittent crashes**: The crash was timing-dependent (only when stack was full)
- **Multi-threading stability**: Each thread has its own stack, but heap is shared safely

## Prevention

To prevent similar issues in the future:
1. **Avoid large local arrays**: Use heap allocation for arrays > 1KB
2. **Code review**: Check for large stack allocations in functions called from deep call stacks
3. **Static analysis**: Use tools to detect potential stack overflow issues
4. **Testing**: Run stress tests with multiple threads to catch stack issues early

