#!/bin/bash
# Stress test script to reproduce early exit issues
# Runs multiple test iterations rapidly to identify patterns

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CTESTER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$CTESTER_DIR"

echo "=== Stress Test: Early Exit Reproduction ==="
echo "Running multiple test iterations to identify patterns..."
echo ""

SUCCESS_COUNT=0
FAIL_COUNT=0
EARLY_EXIT_COUNT=0
DATA_CORRUPTION_COUNT=0
TOTAL_RUNS=10

for i in $(seq 1 $TOTAL_RUNS); do
    echo "--- Test Run $i/$TOTAL_RUNS ---"
    
    # Clean previous results
    rm -f tmp/test_2core_only/test_2core_results.csv
    rm -f tmp/test_2core_only/test_2core.log
    
    # Run test with timeout (increased to 300 seconds - tests take ~2-3 minutes to complete)
    if timeout 300 ./scripts/test_2core_only.sh > /tmp/stress_test_run_${i}.log 2>&1; then
        # Check if test completed successfully
        if grep -q "Main loop COMPLETED" /tmp/stress_test_run_${i}.log; then
            RESULT_COUNT=$(wc -l < tmp/test_2core_only/test_2core_results.csv 2>/dev/null || echo "0")
            if [ "$RESULT_COUNT" -ge "6" ]; then  # Header + 5 results
                echo "  ✓ SUCCESS: Test completed with $((RESULT_COUNT-1)) results"
                SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            else
                echo "  ⚠ EARLY EXIT: Only $((RESULT_COUNT-1)) results (expected 5)"
                EARLY_EXIT_COUNT=$((EARLY_EXIT_COUNT + 1))
            fi
        else
            echo "  ✗ EARLY EXIT: Test did not complete (no COMPLETED message)"
            EARLY_EXIT_COUNT=$((EARLY_EXIT_COUNT + 1))
        fi
        
        # Check for data corruption
        if grep -q "DATA ISSUE\|CRITICAL ERROR.*Invalid time\|Array bounds violation" /tmp/stress_test_run_${i}.log; then
            echo "  ⚠ DATA CORRUPTION detected in run $i"
            DATA_CORRUPTION_COUNT=$((DATA_CORRUPTION_COUNT + 1))
            echo "  First corruption error:"
            grep -m 1 "DATA ISSUE\|CRITICAL ERROR.*Invalid time\|Array bounds violation" /tmp/stress_test_run_${i}.log | head -1
        fi
    else
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 124 ]; then
            echo "  ✗ TIMEOUT: Test exceeded 300 seconds"
        else
            echo "  ✗ FAILED: Test exited with code $EXIT_CODE"
        fi
        FAIL_COUNT=$((FAIL_COUNT + 1))
        
        # Check for data corruption even in failed runs
        if grep -q "DATA ISSUE\|CRITICAL ERROR.*Invalid time\|Array bounds violation" /tmp/stress_test_run_${i}.log; then
            echo "  ⚠ DATA CORRUPTION detected in failed run $i"
            DATA_CORRUPTION_COUNT=$((DATA_CORRUPTION_COUNT + 1))
        fi
    fi
    
    # Small delay between runs
    sleep 1
done

echo ""
echo "=== Stress Test Summary ==="
echo "Total runs: $TOTAL_RUNS"
echo "Successful: $SUCCESS_COUNT"
echo "Early exits: $EARLY_EXIT_COUNT"
echo "Failures: $FAIL_COUNT"
echo "Data corruption incidents: $DATA_CORRUPTION_COUNT"
echo ""
echo "Success rate: $((SUCCESS_COUNT * 100 / TOTAL_RUNS))%"
echo "Early exit rate: $((EARLY_EXIT_COUNT * 100 / TOTAL_RUNS))%"
echo "Data corruption rate: $((DATA_CORRUPTION_COUNT * 100 / TOTAL_RUNS))%"
echo ""
echo "Detailed logs saved in /tmp/stress_test_run_*.log"

