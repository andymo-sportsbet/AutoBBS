#!/bin/bash
# Test script for Phase 2.2: Thread-Local Logging Testing and Validation
# Tests thread-local logging with different thread counts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

CONFIG_FILE="config/Peso_MACD_BTCUSD-1H_860013_optimize.config"
TEST_RESULTS_DIR="tmp/thread_local_logging_tests"
mkdir -p "$TEST_RESULTS_DIR"
mkdir -p log

echo "=== Phase 2.2: Thread-Local Logging Testing ==="
echo ""

# Function to run test with specific numcores
run_test() {
    local numcores=$1
    local test_name=$2
    local temp_config="${TEST_RESULTS_DIR}/test_${test_name}_$$.config"
    
    echo "--- Test: $test_name (numcores=$numcores) ---"
    
    # Create temporary config with specified numcores
    cp "$CONFIG_FILE" "$temp_config"
    
    # Update numcores
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^numcores = .*/numcores = ${numcores}/" "$temp_config"
    else
        sed -i "s/^numcores = .*/numcores = ${numcores}/" "$temp_config"
    fi
    
    # Update logseverity to INFO (6) for better visibility
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^logseverity = .*/logseverity = 6/" "$temp_config"
    else
        sed -i "s/^logseverity = .*/logseverity = 6/" "$temp_config"
    fi
    
    # Clean log directory before test
    rm -f log/AsirikuyFramework*.log log/AsirikuyCTester*.log
    
    echo "  Running optimization with numcores=$numcores..."
    echo "  Config: $temp_config"
    echo "  Log directory: log/"
    
    # Update date range to a shorter period for faster testing
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^fromdate = .*/fromdate = 2023-01-01/" "$temp_config"
        sed -i '' "s/^todate = .*/todate = 2023-06-01/" "$temp_config"
    else
        sed -i "s/^fromdate = .*/fromdate = 2023-01-01/" "$temp_config"
        sed -i "s/^todate = .*/todate = 2023-06-01/" "$temp_config"
    fi
    
    # Run optimization (with timeout for safety)
    set +e
    timeout 600 python3 asirikuy_strategy_tester.py -c "$temp_config" -ot results -oo "${TEST_RESULTS_DIR}/test_${test_name}_results" 2>&1 | tee "${TEST_RESULTS_DIR}/test_${test_name}.log"
    TEST_EXIT_CODE=$?
    set -e
    
    echo ""
    echo "  Test completed (exit code: $TEST_EXIT_CODE)"
    
    # Check log files
    echo "  Checking log files..."
    ls -lh log/AsirikuyFramework*.log log/AsirikuyCTester*.log 2>/dev/null || echo "  No log files found"
    
    # Save log files for review
    if ls log/AsirikuyFramework*.log log/AsirikuyCTester*.log 1>/dev/null 2>&1; then
        cp log/AsirikuyFramework*.log log/AsirikuyCTester*.log "${TEST_RESULTS_DIR}/" 2>/dev/null || true
        echo "  ✓ Log files saved to ${TEST_RESULTS_DIR}/"
    fi
    
    echo ""
    
    # Return exit code
    return $TEST_EXIT_CODE
}

# Test 1: Single thread (backward compatibility)
echo "Test 1: Single Thread (numcores=1) - Backward Compatibility"
run_test 1 "single_thread" || echo "  ⚠️  Test 1 had issues (check log)"
echo ""

# Test 2: 2 threads
echo "Test 2: Two Threads (numcores=2) - Thread-Local Logging"
run_test 2 "two_threads" || echo "  ⚠️  Test 2 had issues (check log)"
echo ""

# Test 3: 4 threads
echo "Test 3: Four Threads (numcores=4) - Thread-Local Logging"
run_test 4 "four_threads" || echo "  ⚠️  Test 3 had issues (check log)"
echo ""

echo "=== Testing Complete ==="
echo "Results saved to: ${TEST_RESULTS_DIR}/"
echo ""
echo "To review results:"
echo "  ls -lh ${TEST_RESULTS_DIR}/"
echo "  cat ${TEST_RESULTS_DIR}/test_*.log"

