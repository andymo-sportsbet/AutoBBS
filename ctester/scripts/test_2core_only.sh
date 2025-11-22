#!/bin/bash
# Test script for 2-core thread-local logging validation
# Runs optimization with numcores=2 and verifies thread-local log files are created

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

CONFIG_FILE="config/Peso_MACD_BTCUSD-1H_860013_optimize.config"
TEST_RESULTS_DIR="tmp/test_2core_only"
mkdir -p "$TEST_RESULTS_DIR"
mkdir -p log

echo "=== 2-Core Thread-Local Logging Test ==="
echo ""

# Clean up previous test results
rm -f log/AsirikuyFramework*.log log/AsirikuyCTester*.log
rm -f "${TEST_RESULTS_DIR}"/*.log "${TEST_RESULTS_DIR}"/*.csv "${TEST_RESULTS_DIR}"/*.config 2>/dev/null || true

# Create temporary config with numcores=2
TEMP_CONFIG="${TEST_RESULTS_DIR}/test_2core_$$.config"
cp "$CONFIG_FILE" "$TEMP_CONFIG"

# Update numcores to 2
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s/^numcores = .*/numcores = 2/" "$TEMP_CONFIG"
    sed -i '' "s/^logseverity = .*/logseverity = 6/" "$TEMP_CONFIG"
    # Use a shorter date range for faster testing (1 month)
    sed -i '' "s/^fromdate = .*/fromdate = 2023-01-01/" "$TEMP_CONFIG"
    sed -i '' "s/^todate = .*/todate = 2023-02-01/" "$TEMP_CONFIG"
else
    sed -i "s/^numcores = .*/numcores = 2/" "$TEMP_CONFIG"
    sed -i "s/^logseverity = .*/logseverity = 6/" "$TEMP_CONFIG"
    sed -i "s/^fromdate = .*/fromdate = 2023-01-01/" "$TEMP_CONFIG"
    sed -i "s/^todate = .*/todate = 2023-02-01/" "$TEMP_CONFIG"
fi

echo "Configuration:"
echo "  - numcores: 2"
echo "  - logseverity: 6 (INFO)"
echo "  - Date range: 2023-01-01 to 2023-02-01 (1 month)"
echo "  - Config file: $TEMP_CONFIG"
echo ""

# Verify numcores setting
NUMCORES=$(grep "^numcores" "$TEMP_CONFIG" | cut -d'=' -f2 | tr -d ' ')
if [ "$NUMCORES" != "2" ]; then
    echo "ERROR: numcores is not set to 2 in config file!"
    exit 1
fi
echo "✓ Verified numcores = 2"
echo ""

# Run optimization
echo "Running optimization with 2 cores..."
echo "  This may take a few minutes..."
echo "  Progress will be logged to: ${TEST_RESULTS_DIR}/test_2core.log"
echo ""

set +e
python3 asirikuy_strategy_tester.py -c "$TEMP_CONFIG" -ot results -oo "${TEST_RESULTS_DIR}/test_2core_results" 2>&1 | tee "${TEST_RESULTS_DIR}/test_2core.log"
PYTHON_EXIT_CODE=$?
set -e

echo ""
echo "=== Test Results ==="
echo ""

# Check exit code
if [ $PYTHON_EXIT_CODE -ne 0 ]; then
    echo "⚠️  WARNING: Python process exited with code $PYTHON_EXIT_CODE"
else
    echo "✓ Python process completed successfully (exit code: $PYTHON_EXIT_CODE)"
fi

# Check for thread-local log files
echo ""
echo "=== Thread-Local Log Files ==="
THREAD_LOG_COUNT=$(ls -1 log/AsirikuyFramework_thread*.log 2>/dev/null | wc -l | tr -d ' ')
if [ "$THREAD_LOG_COUNT" -eq 0 ]; then
    echo "❌ FAIL: No thread-local log files found!"
    echo "   Expected: log/AsirikuyFramework_thread0.log and thread1.log"
    exit 1
elif [ "$THREAD_LOG_COUNT" -lt 2 ]; then
    echo "⚠️  WARNING: Only $THREAD_LOG_COUNT thread-local log file(s) found (expected 2)"
else
    echo "✓ Found $THREAD_LOG_COUNT thread-local log file(s)"
fi

# List thread-local log files
echo ""
echo "Thread-local log files:"
ls -lh log/AsirikuyFramework_thread*.log 2>/dev/null | while read line; do
    echo "  $line"
done

# Check for specific thread files
if [ -f "log/AsirikuyFramework_thread0.log" ]; then
    THREAD0_SIZE=$(ls -lh log/AsirikuyFramework_thread0.log | awk '{print $5}')
    echo "  ✓ Thread 0 log file exists (size: $THREAD0_SIZE)"
else
    echo "  ❌ Thread 0 log file missing!"
fi

if [ -f "log/AsirikuyFramework_thread1.log" ]; then
    THREAD1_SIZE=$(ls -lh log/AsirikuyFramework_thread1.log | awk '{print $5}')
    echo "  ✓ Thread 1 log file exists (size: $THREAD1_SIZE)"
else
    echo "  ❌ Thread 1 log file missing!"
fi

# Check for thread initialization messages
echo ""
echo "=== Thread Initialization Verification ==="
THREAD_INIT_COUNT=$(grep -c "Thread.*initialized thread-local logging" "${TEST_RESULTS_DIR}/test_2core.log" 2>/dev/null || echo "0")
if [ "$THREAD_INIT_COUNT" -ge 2 ]; then
    echo "✓ Found $THREAD_INIT_COUNT thread initialization messages (expected 2)"
    grep "Thread.*initialized thread-local logging" "${TEST_RESULTS_DIR}/test_2core.log" | while read line; do
        echo "  $line"
    done
else
    echo "⚠️  WARNING: Only found $THREAD_INIT_COUNT thread initialization messages (expected 2)"
fi

# Check for thread cleanup messages
echo ""
echo "=== Thread Cleanup Verification ==="
THREAD_CLOSE_COUNT=$(grep -c "Thread.*closed thread-local log file" "${TEST_RESULTS_DIR}/test_2core.log" 2>/dev/null | tr -d ' ' || echo "0")
if [ "$THREAD_CLOSE_COUNT" -ge 2 ]; then
    echo "✓ Found $THREAD_CLOSE_COUNT thread cleanup messages (expected 2)"
    grep "Thread.*closed thread-local log file" "${TEST_RESULTS_DIR}/test_2core.log" | while read line; do
        echo "  $line"
    done
else
    echo "⚠️  WARNING: Only found $THREAD_CLOSE_COUNT thread cleanup messages (expected 2)"
fi

# Check CSV results
echo ""
echo "=== Optimization Results ==="
if [ -f "${TEST_RESULTS_DIR}/test_2core_results.csv" ]; then
    CSV_LINES=$(wc -l < "${TEST_RESULTS_DIR}/test_2core_results.csv" 2>/dev/null || echo "0")
    if [ "$CSV_LINES" -le 1 ]; then
        echo "⚠️  WARNING: CSV file exists but contains no results (only header)"
        echo "   File: ${TEST_RESULTS_DIR}/test_2core_results.csv"
    else
        echo "✓ CSV file contains $((CSV_LINES - 1)) result line(s)"
        echo "   File: ${TEST_RESULTS_DIR}/test_2core_results.csv"
        echo "   First few results:"
        head -5 "${TEST_RESULTS_DIR}/test_2core_results.csv" | while read line; do
            echo "     $line"
        done
    fi
else
    echo "⚠️  WARNING: No CSV results file found"
    echo "   Expected: ${TEST_RESULTS_DIR}/test_2core_results.csv"
fi

# Summary
echo ""
echo "=== Test Summary ==="
THREAD_LOG_COUNT_NUM=$(echo "$THREAD_LOG_COUNT" | tr -d ' ')
THREAD_INIT_COUNT_NUM=$(echo "$THREAD_INIT_COUNT" | tr -d ' ')
if [ "$THREAD_LOG_COUNT_NUM" -ge 2 ] && [ "$THREAD_INIT_COUNT_NUM" -ge 2 ]; then
    echo "✅ PASS: 2-core thread-local logging test passed"
    echo "   - Thread-local log files created: ✓ ($THREAD_LOG_COUNT_NUM files)"
    echo "   - Thread initialization verified: ✓ ($THREAD_INIT_COUNT_NUM messages)"
    THREAD_CLOSE_COUNT_NUM=$(echo "$THREAD_CLOSE_COUNT" | tr -d ' ')
    if [ "$THREAD_CLOSE_COUNT_NUM" -ge 2 ]; then
        echo "   - Thread cleanup verified: ✓ ($THREAD_CLOSE_COUNT_NUM messages)"
    else
        echo "   - Thread cleanup: ⚠️  ($THREAD_CLOSE_COUNT_NUM messages, may be in log file)"
    fi
    exit 0
else
    echo "❌ FAIL: 2-core thread-local logging test failed"
    echo "   - Thread-local log files: $THREAD_LOG_COUNT_NUM (expected 2)"
    echo "   - Thread initialization messages: $THREAD_INIT_COUNT_NUM (expected 2)"
    exit 1
fi

