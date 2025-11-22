# Thread-Local Logging Hotfix - Task List

**Branch**: `hotfix/thread-local-logging`  
**Priority**: High (Performance Critical)  
**Status**: In Progress

## Status Legend
- ⬜ Pending
- 🔄 In Progress
- ✅ Completed
- ❌ Blocked
- ⚠️ At Risk
- 🔍 Review Needed

---

## Phase 1: Core Logger Implementation

### 1.1 Thread-Local Storage Infrastructure

- [✅] Add thread-local storage variables to `AsirikuyLogger.c`
  - [✅] Define `__thread FILE* threadLocalLogFile` for macOS/Linux
  - [✅] Define `__declspec(thread) FILE* threadLocalLogFile` for Windows
  - [✅] Define `__thread int threadLocalSeverityLevel` for severity tracking
  - [✅] Add fallback implementation for platforms without thread-local support

- [✅] Implement `asirikuyLoggerInitThreadLocal()` function
  - [✅] Create function in `AsirikuyLogger.c`
  - [✅] Handle log file path validation
  - [✅] Create directory if needed
  - [✅] Open thread-local log file in append mode
  - [✅] Write log file header with thread ID
  - [✅] Set thread-local severity level
  - [✅] Add error handling and logging

- [✅] Implement `asirikuyLoggerCloseThreadLocal()` function
  - [✅] Create function in `AsirikuyLogger.c`
  - [✅] Close thread-local log file if open
  - [✅] Reset thread-local severity level
  - [✅] Add error handling

- [✅] Update `AsirikuyLogger.h` header
  - [✅] Add function declaration for `asirikuyLoggerInitThreadLocal()`
  - [✅] Add function declaration for `asirikuyLoggerCloseThreadLocal()`
  - [✅] Add documentation comments

### 1.2 Modify `asirikuyLogMessage()` Function

- [✅] Refactor `asirikuyLogMessage()` to support thread-local logging
  - [✅] Move message formatting before critical section check
  - [✅] Add thread-local logger check (before critical section)
  - [✅] Write to thread-local file if available (no lock needed)
  - [✅] Return early if thread-local logging succeeds
  - [✅] Keep global logger fallback (with critical section)
  - [✅] Ensure backward compatibility

- [✅] Add OpenMP support detection
  - [✅] Include `<omp.h>` if `_OPENMP` is defined
  - [✅] Use `omp_get_thread_num()` for thread ID in log headers

---

## Phase 2: Optimizer Integration

### 2.1 Initialize Thread-Local Logging in Parallel Loop

- [✅] Modify `optimizer.c` OpenMP parallel region
  - [✅] Locate the `#pragma omp parallel for` section
  - [✅] Add thread-local logging initialization at start of loop
  - [✅] Generate thread-specific log file paths
  - [✅] Call `asirikuyLoggerInitThreadLocal()` for each thread (only when numThreads > 1)
  - [✅] Add debug logging for initialization

- [✅] Implement log file path generation
  - [✅] Determine log folder location (from config or default)
  - [✅] Generate path: `{logFolder}/AsirikuyFramework_thread{N}.log`
  - [✅] Handle path length limits
  - [✅] Ensure directory exists

- [⬜] Add cleanup (optional)
  - [⬜] Consider calling `asirikuyLoggerCloseThreadLocal()` at end
  - [⬜] Or let threads clean up automatically

### 2.2 Testing and Validation

- [⬜] Test with single thread
  - [⬜] Verify backward compatibility
  - [⬜] Verify global logger still works
  - [⬜] Check log file creation

- [⬜] Test with multiple threads
  - [⬜] Run with 2 threads
  - [⬜] Run with 4 threads
  - [⬜] Verify separate log files created
  - [⬜] Verify no log corruption

- [⬜] Performance testing
  - [⬜] Measure optimization execution time before
  - [⬜] Measure optimization execution time after
  - [⬜] Compare critical section contention (use profiling)
  - [⬜] Verify performance improvement

---

## Phase 3: Reduce Hot Path Logging (Optional Performance Enhancement)

### 3.1 Identify Hot Path Log Calls

- [⬜] Review `tester.c` for frequent log calls
  - [⬜] Line 1380: `logInfo()` in main loop
  - [⬜] Line 1401: `logInfo()` per system per iteration
  - [⬜] Identify other high-frequency log calls

### 3.2 Optimize Logging Frequency

- [⬜] Remove or reduce frequency of hot path logs
  - [⬜] Option 1: Remove unnecessary logs entirely
  - [⬜] Option 2: Log only every N iterations (e.g., every 1000)
  - [⬜] Option 3: Use conditional compilation flag
  - [⬜] Keep important logs (errors, completion, milestones)

- [⬜] Add progress logging at milestones
  - [⬜] Log every 10% of iterations
  - [⬜] Log every 30 seconds (time-based)
  - [⬜] Log at key events (test start, completion, errors)

---

## Phase 4: Testing and Validation

### 4.1 Unit Tests

- [⬜] Test thread-local storage initialization
- [⬜] Test thread-local logging writes
- [⬜] Test thread-local cleanup
- [⬜] Test fallback to global logger
- [⬜] Test error handling

### 4.2 Integration Tests

- [⬜] Run optimization with thread-local logging enabled
- [⬜] Verify all threads create their own log files
- [⬜] Verify log file content is correct
- [⬜] Verify no missing log entries
- [⬜] Verify no log corruption

### 4.3 Performance Tests

- [⬜] Benchmark log call latency
  - [⬜] Measure before (with critical section)
  - [⬜] Measure after (with thread-local)
  - [⬜] Calculate improvement percentage

- [⬜] Benchmark optimization execution time
  - [⬜] Run full optimization before
  - [⬜] Run full optimization after
  - [⬜] Compare total execution time
  - [⬜] Compare per-iteration time

- [⬜] Profile critical section contention
  - [⬜] Use profiling tools (e.g., `perf`, `Instruments`)
  - [⬜] Measure lock wait times before
  - [⬜] Measure lock wait times after
  - [⬜] Verify contention eliminated

### 4.4 Regression Tests

- [⬜] Verify backward compatibility
  - [⬜] Single-threaded execution still works
  - [⬜] Global logger still works when thread-local not initialized
  - [⬜] Existing log files still readable

- [⬜] Verify no memory leaks
  - [⬜] Run with valgrind or similar tool
  - [⬜] Check for file handle leaks
  - [⬜] Check for memory leaks

---

## Phase 5: Tmp File Thread-Safety Fixes

### 5.1 Fix `results.open` File (CRITICAL)

- [✅] Modify `save_openorder_to_file()` function
  - [✅] Add `testId` and `instanceId` parameters to function signature
  - [✅] Change hardcoded filename `"results.open"` to `"results_{testId}.open"` (backtesting) or `"results_{instanceId}.open"` (live trading)
  - [✅] Update function call in `tester.c:1960` to pass both `testId` and `instanceId`
  - [⬜] Verify thread-specific files are created correctly (testing pending)

### 5.2 Fix InstanceId Uniqueness (HIGH)

- [✅] Fix instanceId formula in `optimizer.c`
  - [✅] Review current formula: `(testId+1) + 2*(n+1)`
  - [✅] Identify collision issues (verified: 24 collisions with 8 threads, 5 symbols)
  - [✅] Implement collision-free formula: `(testId * 1000) + n + 1`
  - [✅] Update both `testFitnessMultipleSymbols()` and `runOptimizationMultipleSymbols()`
  - [✅] Verified uniqueness: 0 collisions with 8 threads, 100 symbols (800 combinations)
  - [⬜] Test with multiple threads to verify uniqueness in practice (testing pending)

### 5.3 Fix `{instanceId}_OrderInfo.txt` Thread-Safety (HIGH)

- [✅] Modify `saveTradingInfo()` in `StrategyUserInterface.c`
  - [✅] Added `CriticalSection.h` include
  - [✅] Added critical section around file write operations (defense-in-depth)
  - [✅] Proper error handling with return code
  - [⬜] Test with multiple threads to verify no corruption (testing pending)

### 5.4 Fix `{instanceId}.state` File Thread-Safety (MEDIUM)

- [✅] Review `backupInstanceState()` in `InstanceStates.c`
  - [✅] Verified critical section protection (already protected by `hasInstanceRunOnCurrentBar()`)
  - [✅] File writes are already thread-safe (called within `enterCriticalSection()` / `leaveCriticalSection()`)
  - [⬜] Test with multiple threads to verify no corruption (testing pending)

### 5.5 Testing Tmp File Thread-Safety

- [⬜] Run optimization with 4+ threads
- [⬜] Verify no file corruption in `tmp` directory
- [⬜] Verify all expected files are created
- [⬜] Verify no duplicate filenames
- [⬜] Check file contents for corruption
- [⬜] Verify `results_{testId}.open` files are created correctly

---

## Phase 6: Documentation and Deployment

### 6.1 Code Documentation

- [⬜] Add inline comments to new functions
- [⬜] Document thread-local storage usage
- [⬜] Document log file naming convention
- [⬜] Document backward compatibility behavior
- [⬜] Document tmp file thread-safety fixes

### 6.2 User Documentation

- [⬜] Update README with thread-local logging info
- [⬜] Document log file locations
- [⬜] Document how to find thread-specific logs
- [⬜] Document tmp file naming conventions
- [⬜] Add troubleshooting section

### 6.3 Deployment

- [⬜] Merge to `refactor` branch
- [⬜] Run production optimization tests
- [⬜] Monitor performance improvements
- [⬜] Verify tmp file thread-safety in production
- [⬜] Collect user feedback

---

## Current Status Summary

**Overall Progress**: 30% (18/60 tasks completed)

**Phase 1**: 100% (12/12 tasks) ✅  
**Phase 2**: 33% (2/6 tasks) 🔄  
**Phase 3**: 0% (0/4 tasks)  
**Phase 4**: 0% (0/18 tasks)  
**Phase 5**: 80% (8/10 tasks) ✅ **CRITICAL FIXES COMPLETE**  
**Phase 6**: 0% (0/10 tasks)

**Next Steps**:
1. ✅ Phase 1 Complete: Thread-local storage and logging functions implemented
2. ✅ Phase 2.1 Complete: Thread-local logging initialization in optimizer
3. ✅ Phase 5 Complete: Tmp file thread-safety fixes implemented
   - ✅ Fixed `results.open` hardcoded filename (CRITICAL)
   - ✅ Fixed instanceId collision formula (HIGH)
   - ✅ Fixed OrderInfo.txt thread-safety (HIGH)
   - ✅ Verified .state file thread-safety (MEDIUM)
4. 🔄 **NEXT**: Phase 5.5 - Test tmp file thread-safety with multiple threads
5. 🔄 Phase 2.2 - Test thread-local logging with single thread (backward compatibility)
6. 🔄 Phase 2.2 - Test thread-local logging with multiple threads (2, 4, 8)
7. 🔄 Phase 4.3 - Measure performance improvement

**Blockers**: None

**Risks**:
- ⚠️ Thread-local storage may not be available on all platforms (mitigation: fallback implementation)
- ⚠️ Too many log files may be created (mitigation: configurable naming, optional cleanup)
- ⚠️ **CRITICAL**: Tmp files (`results.open`, `{instanceId}_OrderInfo.txt`, `{instanceId}.state`) are not thread-safe and can cause file corruption in multi-threaded runs (mitigation: Phase 5 fixes)

---

## Notes

- This hotfix addresses a critical performance issue affecting optimization runs
- Thread-local logging eliminates critical section contention
- Backward compatibility is maintained through fallback to global logger
- Performance improvement target: 50%+ faster optimization execution
- **NEW**: Tmp file thread-safety issues discovered and documented in `TMP_FILES_THREAD_SAFETY.md`
  - `results.open`: Hardcoded filename shared across threads (CRITICAL)
  - `{instanceId}_OrderInfo.txt`: InstanceId collisions cause file corruption (HIGH)
  - `{instanceId}.state`: File writes not protected (MEDIUM)

**Last Updated**: November 2024

