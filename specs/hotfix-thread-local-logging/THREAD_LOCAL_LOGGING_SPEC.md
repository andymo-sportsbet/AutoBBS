# Thread-Local Logging Hotfix Specification

**Date**: November 2024  
**Status**: Draft  
**Priority**: High (Performance Critical)  
**Branch**: `hotfix/thread-local-logging`

## Executive Summary

This document specifies the implementation of thread-local logging to eliminate critical section contention in multi-threaded optimization runs. The current shared logger implementation causes severe performance degradation when multiple OpenMP threads compete for the same critical section during frequent logging operations.

**Problem**: With 4 threads running parallel optimizations, each thread logs ~34,935 times per test iteration, resulting in ~139,740 log calls competing for a single critical section. This causes:
- Severe performance degradation (3-4 minute pauses)
- Thread serialization instead of parallel execution
- Unpredictable execution times

**Solution**: Implement thread-local storage for log files, allowing each thread to write to its own log file without contention.

---

## 1. Problem Analysis

### 1.1 Current Implementation Issues

**Location**: `core/AsirikuyCommon/src/AsirikuyLogger.c`

**Current Flow**:
1. All threads call `asirikuyLogMessage()` 
2. Each call acquires `enterCriticalSection()` (line 206)
3. Threads queue up waiting for the lock
4. One thread formats message, writes to file, flushes
5. Releases lock, next thread proceeds
6. **Result**: Serial execution instead of parallel

**Hot Path Logging**:
- `tester.c` line 1380: `logInfo()` called on **every loop iteration**
- `tester.c` line 1401: `logInfo()` called for **every system on every iteration**
- With 34,935 loop iterations × 4 threads = **139,740 log calls per test**

### 1.2 Performance Impact

**Observed Symptoms**:
- Optimization runs are "very slow" and "sometimes fast"
- 3-4 minute pauses in log file updates
- High CPU usage (161%) but slow progress
- Inconsistent execution times

**Root Cause**: Critical section contention causing thread serialization

---

## 2. Solution Design

### 2.1 Thread-Local Storage Approach

**Strategy**: Use compiler-supported thread-local storage (`__thread` on GCC/Clang, `_Thread_local` on C11, `__declspec(thread)` on MSVC) to give each thread its own log file handle.

**Benefits**:
- ✅ Zero contention (no locks needed for thread-local writes)
- ✅ Better performance (parallel file I/O)
- ✅ Easier debugging (separate log files per thread)
- ✅ Backward compatible (falls back to global logger if not initialized)

### 2.2 Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    asirikuyLogMessage()                  │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
              ┌───────────────────────┐
              │ Check Thread-Local    │
              │ Logger Initialized?   │
              └───────────────────────┘
                      │
          ┌───────────┴───────────┐
          │                       │
         YES                     NO
          │                       │
          ▼                       ▼
┌──────────────────┐    ┌──────────────────┐
│ Write to Thread- │    │ Acquire Critical│
│ Local File       │    │ Section         │
│ (NO LOCK)        │    │                 │
└──────────────────┘    │ Write to Global │
                        │ Log Files       │
                        │ (WITH LOCK)     │
                        └──────────────────┘
```

### 2.3 Implementation Details

**New Functions**:
- `asirikuyLoggerInitThreadLocal(const char* pLogFilePath, int severityLevel)` - Initialize thread-local logger
- `asirikuyLoggerCloseThreadLocal(void)` - Clean up thread-local logger

**Thread-Local Variables**:
- `__thread FILE* threadLocalLogFile` - Per-thread log file handle
- `__thread int threadLocalSeverityLevel` - Per-thread severity level

**Modified Function**:
- `asirikuyLogMessage()` - Check thread-local first, fall back to global

---

## 3. Implementation Plan

### 3.1 Phase 1: Core Logger Changes

**File**: `core/AsirikuyCommon/src/AsirikuyLogger.c`

**Changes**:
1. Add thread-local storage variables
2. Implement `asirikuyLoggerInitThreadLocal()`
3. Implement `asirikuyLoggerCloseThreadLocal()`
4. Modify `asirikuyLogMessage()` to check thread-local first

**File**: `core/AsirikuyCommon/include/AsirikuyLogger.h`

**Changes**:
1. Add function declarations for thread-local logging

### 3.2 Phase 2: Optimizer Integration

**File**: `core/CTesterFrameworkAPI/src/optimizer.c`

**Changes**:
1. Initialize thread-local logging in OpenMP parallel loop
2. Generate thread-specific log file paths (e.g., `log/AsirikuyFramework_thread0.log`)
3. Optional: Clean up thread-local logging at end of iteration

### 3.3 Phase 3: Reduce Hot Path Logging (Optional)

**File**: `core/CTesterFrameworkAPI/src/tester.c`

**Changes**:
1. Remove or reduce frequency of `logInfo()` calls on lines 1380 and 1401
2. Only log every N iterations (e.g., every 1000 iterations)

---

## 4. Technical Specifications

### 4.1 Thread-Local Storage Implementation

**Platform Support**:
- **macOS/Linux**: `__thread` (GCC/Clang extension)
- **Windows**: `__declspec(thread)` (MSVC)
- **C11 Standard**: `_Thread_local` (if compiler supports)

**Fallback**: If thread-local storage not available, use per-thread array indexed by thread ID (limited to MAX_THREADS=64)

### 4.2 Log File Naming Convention

**Pattern**: `{logFolder}/AsirikuyFramework_thread{N}.log`

**Examples**:
- `log/AsirikuyFramework_thread0.log`
- `log/AsirikuyFramework_thread1.log`
- `log/AsirikuyFramework_thread2.log`
- `log/AsirikuyFramework_thread3.log`

### 4.3 Thread Safety

**Thread-Local Operations**: No synchronization needed (each thread has its own storage)

**Global Logger Fallback**: Still uses critical section (backward compatible)

**Initialization**: Thread-local initialization should happen once per thread, typically at the start of the OpenMP parallel region

---

## 5. Testing Requirements

### 5.1 Functional Tests

- [ ] Single-threaded logging still works (backward compatibility)
- [ ] Multi-threaded logging creates separate log files per thread
- [ ] Thread-local logging bypasses critical section
- [ ] Global logger fallback works when thread-local not initialized
- [ ] Log file rotation/cleanup works correctly

### 5.2 Performance Tests

- [ ] Measure log call latency (should be significantly reduced)
- [ ] Measure optimization execution time (should be faster)
- [ ] Verify no critical section contention (use profiling tools)
- [ ] Compare before/after performance metrics

### 5.3 Integration Tests

- [ ] Run optimization with 1, 2, 4, 8 threads
- [ ] Verify all threads write to their own log files
- [ ] Verify log files contain correct thread-specific content
- [ ] Verify no log corruption or missing entries

---

## 6. Rollout Plan

### 6.1 Development

1. Implement thread-local logging in `AsirikuyLogger.c`
2. Add initialization in `optimizer.c`
3. Test with small optimization runs
4. Measure performance improvement

### 6.2 Testing

1. Run full optimization suite
2. Verify log file integrity
3. Compare performance metrics
4. Check for regressions

### 6.3 Deployment

1. Merge to `refactor` branch
2. Run production optimization tests
3. Monitor performance improvements
4. Document log file locations

---

## 7. Success Criteria

### 7.1 Performance

- ✅ Optimization runs complete faster (target: 50%+ improvement)
- ✅ No more 3-4 minute pauses in log updates
- ✅ Consistent execution times across runs
- ✅ CPU utilization remains high but progress is faster

### 7.2 Functionality

- ✅ All log messages are captured
- ✅ Log files are properly organized by thread
- ✅ No log corruption or missing entries
- ✅ Backward compatibility maintained

### 7.3 Code Quality

- ✅ Thread-safe implementation
- ✅ No memory leaks
- ✅ Proper error handling
- ✅ Well-documented code

---

## 8. Risks and Mitigation

### 8.1 Risks

**Risk 1**: Thread-local storage not available on all platforms
- **Mitigation**: Provide fallback implementation using per-thread array

**Risk 2**: Too many log files created
- **Mitigation**: Use configurable log file naming, optional cleanup

**Risk 3**: Log file I/O still slow (disk bottleneck)
- **Mitigation**: Consider buffered writes, async I/O in future

**Risk 4**: Breaking existing logging behavior
- **Mitigation**: Maintain backward compatibility, fall back to global logger

### 8.2 Dependencies

- OpenMP support (already present)
- Thread-local storage support (compiler-dependent)
- File system write permissions

---

## 9. Future Enhancements

### 9.1 Potential Improvements

1. **Buffered Logging**: Batch log writes to reduce I/O calls
2. **Async Logging**: Use background thread for log file writes
3. **Log Aggregation**: Merge thread logs into single file post-execution
4. **Configurable Logging**: Allow users to choose thread-local vs. global
5. **Log Rotation**: Automatic cleanup of old thread log files

### 9.2 Performance Monitoring

- Add metrics for log call latency
- Track critical section wait times
- Monitor file I/O performance
- Profile optimization execution times

---

## 10. References

- **Current Logger**: `core/AsirikuyCommon/src/AsirikuyLogger.c`
- **Optimizer**: `core/CTesterFrameworkAPI/src/optimizer.c`
- **Tester**: `core/CTesterFrameworkAPI/src/tester.c`
- **OpenMP Documentation**: https://www.openmp.org/
- **Thread-Local Storage**: C11 `_Thread_local`, GCC `__thread`, MSVC `__declspec(thread)`

---

## Appendix A: Code Examples

### A.1 Thread-Local Initialization

```c
// In optimizer.c, inside OpenMP parallel loop
#ifdef _OPENMP
int thread_id = omp_get_thread_num();
char threadLogPath[512];
snprintf(threadLogPath, sizeof(threadLogPath), 
         "log/AsirikuyFramework_thread%d.log", thread_id);
asirikuyLoggerInitThreadLocal(threadLogPath, gSeverityLevel);
#endif
```

### A.2 Thread-Local Logging

```c
// In AsirikuyLogger.c
void asirikuyLogMessage(int severity, const char* format, ...)
{
    // ... format message ...
    
    // Check thread-local first (no lock!)
    if(threadLocalSeverityLevel >= 0 && severity <= threadLocalSeverityLevel)
    {
        if(threadLocalLogFile != NULL)
        {
            fprintf(threadLocalLogFile, "%s", logLine);
            fflush(threadLocalLogFile);
            return; // Early return, no critical section needed!
        }
    }
    
    // Fall back to global logger (requires lock)
    enterCriticalSection();
    // ... existing global logging code ...
    leaveCriticalSection();
}
```

---

**Document Version**: 1.0  
**Last Updated**: November 2024  
**Author**: Auto (AI Assistant)  
**Review Status**: Pending

