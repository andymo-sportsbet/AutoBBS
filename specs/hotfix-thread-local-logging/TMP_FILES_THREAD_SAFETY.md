# Thread Safety Analysis: tmp Directory Files

## Overview
This document analyzes the thread-safety of files created in the `tmp` directory during optimization runs, specifically:
- `{instanceId}_OrderInfo.txt` files
- `{instanceId}.state` files  
- `results.open` file

## Files Found in tmp Directory
- `1.state`, `3.state`, `4.state`, `5.state`, `6.state` (binary files)
- `223456_OrderInfo.txt` (text file)

## Analysis

### 1. `results.open` File (CRITICAL: NOT THREAD-SAFE)

**Location**: `core/CTesterFrameworkAPI/src/tester.c:723-738`

**Function**: `save_openorder_to_file()`

**Issue**: 
- Writes to a **hardcoded filename** `"results.open"` (line 730)
- No thread identification in filename
- Called from `runPortfolioTest()` at line 1949 when `orderIndex > 0`
- Multiple threads can write to the same file simultaneously, causing:
  - File corruption
  - Lost data
  - Race conditions

**Current Code**:
```c
void save_openorder_to_file(){
    FILE* openOrderFile;	
    char   timeString[MAX_TIME_STRING_SIZE] = "";
    int orderIndex;
    int index;
    
    openOrderFile = fopen("results.open", "w");  // ❌ HARDCODED, SHARED
    if (openOrderFile == NULL)
        logError("Failed to save openOrderFile.");

    fprintf(openOrderFile, "Open order exist!");

    fclose(openOrderFile);
}
```

**Recommendation**: 
- Make filename thread-specific: `results_{testId}.open` or `results_thread{threadId}.open`
- Or use testId from `runPortfolioTest` parameter

---

### 2. `{instanceId}_OrderInfo.txt` Files (LIKELY SAFE, BUT VERIFY)

**Location**: `core/TradingStrategies/src/StrategyUserInterface.c:411-445`

**Function**: `saveTradingInfo(int instanceID, Order_Info * pOrderInfo)`

**InstanceId Calculation**: 
From `core/CTesterFrameworkAPI/src/optimizer.c:850`:
```c
localSettings[0][STRATEGY_INSTANCE_ID] = (testId+1)+2*(n+1);
```

Where:
- `testId = omp_get_thread_num()` (0-based thread ID in OpenMP parallel mode)
- `n` = symbol index (0-based)

**Uniqueness Analysis**:
- Thread 0, Symbol 0: instanceId = (0+1)+2*(0+1) = **3**
- Thread 0, Symbol 1: instanceId = (0+1)+2*(1+1) = **5**
- Thread 1, Symbol 0: instanceId = (1+1)+2*(0+1) = **4**
- Thread 1, Symbol 1: instanceId = (1+1)+2*(1+1) = **6**
- Thread 2, Symbol 0: instanceId = (2+1)+2*(0+1) = **5** ⚠️ **COLLISION!**

**Issue**: 
- The formula `(testId+1)+2*(n+1)` does **NOT guarantee uniqueness** across threads!
- Example: Thread 0 with Symbol 1 (instanceId=5) and Thread 2 with Symbol 0 (instanceId=5) collide
- This means multiple threads could write to the same `{instanceId}_OrderInfo.txt` file

**Current Code**:
```c
AsirikuyReturnCode saveTradingInfo(int instanceID, Order_Info * pOrderInfo)
{
    char instanceIDName[TOTAL_UI_VALUES];
    char buffer[MAX_FILE_PATH_CHARS] = "";
    char extension[] = "_OrderInfo.txt";
    FILE *fp;

    sprintf(instanceIDName, "%d", instanceID);
    strcat(buffer, tempFilePath);
    strcat(buffer, instanceIDName);
    strcat(buffer, extension);

    fp = fopen(buffer, "w\n");  // ❌ NO LOCK, SHARED IF SAME instanceID
    // ... write operations ...
    fclose(fp);
}
```

**Recommendation**: 
- Use a formula that guarantees uniqueness: `instanceId = (testId * MAX_SYMBOLS) + n + BASE_ID`
- Or add thread ID to filename: `{instanceId}_thread{threadId}_OrderInfo.txt`
- Or add critical section protection around file write

---

### 3. `{instanceId}.state` Files (PARTIALLY SAFE)

**Location**: `core/TradingStrategies/src/InstanceStates.c:143-174`

**Function**: `backupInstanceState(int instanceId)`

**Protection**: 
- Called from `hasInstanceRunOnCurrentBar()` which uses `enterCriticalSection()` / `leaveCriticalSection()`
- However, the file write itself (`fopen`, `fwrite`, `fclose`) is **NOT protected** by the critical section

**Issue**:
- If two threads somehow get the same `instanceId` (see analysis above), they could write to the same `.state` file simultaneously
- The critical section protects the `gInstanceStates` array access, but not the file I/O

**Current Code**:
```c
static void backupInstanceState(int instanceId)
{
  FILE *file;
  char instanceIdString[MAX_FILE_PATH_CHARS] = "";
  char path[MAX_FILE_PATH_CHARS] = "";
  int  instanceIndex = safe_getInstanceIndex(instanceId);  // ✅ Protected by critical section

  // ... path construction ...

  file = fopen(path, "wb");  // ❌ NOT PROTECTED, SHARED IF SAME instanceId
  if(file)
  {
    fwrite(&gInstanceStates[instanceIndex], sizeof(InstanceState), 1, file);
    fclose(file);
  }
}
```

**Recommendation**:
- Fix instanceId uniqueness formula (same as OrderInfo.txt)
- Or add critical section around file write operations
- Or make filename thread-specific

---

## Summary

| File Pattern | Thread-Safe? | Issue | Priority |
|-------------|--------------|-------|----------|
| `results.open` | ❌ **NO** | Hardcoded shared filename | **CRITICAL** |
| `{instanceId}_OrderInfo.txt` | ⚠️ **MAYBE** | InstanceId collision possible | **HIGH** |
| `{instanceId}.state` | ⚠️ **PARTIAL** | File write not protected, instanceId collision possible | **MEDIUM** |

## Recommendations

### Priority 1: Fix `results.open` (CRITICAL)
1. Modify `save_openorder_to_file()` to accept `testId` parameter
2. Change filename to `results_{testId}.open` or `results_thread{threadId}.open`
3. Update call site in `tester.c:1949` to pass `testId`

### Priority 2: Fix InstanceId Uniqueness (HIGH)
1. Change instanceId formula in `optimizer.c:850` to guarantee uniqueness:
   ```c
   // Current: localSettings[0][STRATEGY_INSTANCE_ID] = (testId+1)+2*(n+1);
   // New: Use a formula that guarantees uniqueness across threads
   #define MAX_SYMBOLS_PER_THREAD 10  // Adjust based on max symbols
   localSettings[0][STRATEGY_INSTANCE_ID] = (testId * MAX_SYMBOLS_PER_THREAD) + n + BASE_INSTANCE_ID;
   ```
2. Or add thread ID to filenames: `{instanceId}_thread{threadId}_OrderInfo.txt`

### Priority 3: Add File Write Protection (MEDIUM)
1. Add critical section around file writes in `saveTradingInfo()` and `backupInstanceState()`
2. Or use thread-specific filenames to eliminate need for locks

## Testing

After fixes:
1. Run optimization with 4+ threads
2. Verify no file corruption in `tmp` directory
3. Verify all expected files are created
4. Verify no duplicate filenames
5. Check file contents for corruption

