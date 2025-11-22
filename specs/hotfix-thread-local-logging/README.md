# Thread-Local Logging Hotfix

**Branch**: `hotfix/thread-local-logging`  
**Priority**: High (Performance Critical)  
**Status**: In Progress

## Overview

This hotfix implements thread-local logging to eliminate critical section contention in multi-threaded optimization runs. The current shared logger implementation causes severe performance degradation when multiple OpenMP threads compete for the same critical section during frequent logging operations.

## Problem

With 4 threads running parallel optimizations, each thread logs ~34,935 times per test iteration, resulting in ~139,740 log calls competing for a single critical section. This causes:
- Severe performance degradation (3-4 minute pauses)
- Thread serialization instead of parallel execution
- Unpredictable execution times

## Solution

Implement thread-local storage for log files, allowing each thread to write to its own log file without contention.

## Files

- **Specification**: [THREAD_LOCAL_LOGGING_SPEC.md](./THREAD_LOCAL_LOGGING_SPEC.md)
- **Tasks**: [TASKS.md](./TASKS.md)

## Quick Start

1. Review the [specification](./THREAD_LOCAL_LOGGING_SPEC.md) for detailed design
2. Check [tasks](./TASKS.md) for implementation checklist
3. Implement Phase 1: Core Logger Changes
4. Test with single thread first
5. Test with multiple threads
6. Measure performance improvement

## Key Changes

### Core Logger (`AsirikuyLogger.c`)
- Add thread-local storage variables
- Implement `asirikuyLoggerInitThreadLocal()`
- Implement `asirikuyLoggerCloseThreadLocal()`
- Modify `asirikuyLogMessage()` to check thread-local first

### Optimizer (`optimizer.c`)
- Initialize thread-local logging in OpenMP parallel loop
- Generate thread-specific log file paths

### Optional: Tester (`tester.c`)
- Reduce frequency of hot path log calls

## Success Criteria

- ✅ Optimization runs complete faster (target: 50%+ improvement)
- ✅ No more 3-4 minute pauses in log updates
- ✅ Consistent execution times across runs
- ✅ All log messages captured correctly
- ✅ Backward compatibility maintained

## Related Issues

- Performance degradation in multi-threaded optimization
- Critical section contention in logger
- Slow optimization execution times

## References

- **Current Logger**: `core/AsirikuyCommon/src/AsirikuyLogger.c`
- **Optimizer**: `core/CTesterFrameworkAPI/src/optimizer.c`
- **Tester**: `core/CTesterFrameworkAPI/src/tester.c`

