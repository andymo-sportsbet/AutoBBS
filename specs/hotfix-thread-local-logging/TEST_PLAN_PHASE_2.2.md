# Phase 2.2: Thread-Local Logging Testing Plan

## Test Objectives

1. **Verify backward compatibility** - Single-threaded runs should work exactly as before
2. **Verify thread-local logging** - Multi-threaded runs should create separate log files per thread
3. **Verify no log corruption** - Log files should be properly formatted and complete

## Test Cases

### Test 1: Single Thread (numcores=1) - Backward Compatibility
**Expected Behavior:**
- ✅ No thread-local log files created (`log/AsirikuyFramework_thread*.log` should NOT exist)
- ✅ Global logger files work normally:
  - `log/AsirikuyFramework.log` (if exists)
  - `log/AsirikuyCTester.log` (if exists)
- ✅ Optimization completes successfully
- ✅ All logging goes to global logger (backward compatible)

**Verification Steps:**
1. Run optimization with `numcores=1`
2. Check that no `log/AsirikuyFramework_thread*.log` files exist
3. Check that global log files contain log entries
4. Verify optimization completes successfully

### Test 2: Two Threads (numcores=2) - Thread-Local Logging
**Expected Behavior:**
- ✅ Thread-local log files created:
  - `log/AsirikuyFramework_thread0.log`
  - `log/AsirikuyFramework_thread1.log`
- ✅ Each thread writes to its own log file
- ✅ No log corruption (files are properly formatted)
- ✅ Log files are properly closed after optimization completes
- ✅ Optimization completes successfully

**Verification Steps:**
1. Run optimization with `numcores=2`
2. Check that `log/AsirikuyFramework_thread0.log` and `log/AsirikuyFramework_thread1.log` exist
3. Verify each log file contains entries from its respective thread
4. Check for log corruption (grep for errors, check file integrity)
5. Verify files are properly closed (no file handle leaks)

### Test 3: Four Threads (numcores=4) - Thread-Local Logging
**Expected Behavior:**
- ✅ Thread-local log files created:
  - `log/AsirikuyFramework_thread0.log`
  - `log/AsirikuyFramework_thread1.log`
  - `log/AsirikuyFramework_thread2.log`
  - `log/AsirikuyFramework_thread3.log`
- ✅ Each thread writes to its own log file
- ✅ No log corruption
- ✅ Log files are properly closed after optimization completes
- ✅ Optimization completes successfully

**Verification Steps:**
1. Run optimization with `numcores=4`
2. Check that all 4 thread-local log files exist
3. Verify each log file contains entries from its respective thread
4. Check for log corruption
5. Verify files are properly closed

## Test Configuration

- **Config File**: `ctester/config/Peso_MACD_BTCUSD-1H_860013_optimize.config`
- **Date Range**: 2023-01-01 to 2023-06-01 (shortened for faster testing)
- **Log Severity**: 6 (INFO level) for better visibility
- **Test Script**: `ctester/scripts/test_thread_local_logging.sh`

## Success Criteria

1. ✅ Single-threaded test: No thread-local files, global logger works
2. ✅ Two-threaded test: Two thread-local log files created, no corruption
3. ✅ Four-threaded test: Four thread-local log files created, no corruption
4. ✅ All tests complete successfully
5. ✅ No file handle leaks (files properly closed)

## Test Results

Results will be saved to: `ctester/tmp/thread_local_logging_tests/`

