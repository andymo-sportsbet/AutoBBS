# TradingStrategies Refactoring - Task List

## Overview

This document tracks all tasks for the TradingStrategies refactoring project (Option 1: C++ Internals + C API Wrapper).

**Status Legend:**
- ⬜ Pending
- 🔄 In Progress
- ✅ Completed
- ❌ Blocked
- ⚠️ At Risk

---

## Phase 1: Foundation (Weeks 1-2)

### Week 1: Core Infrastructure

#### Project Setup
- ⬜ Create feature branch: `refactor/cpp-migration`
- ⬜ Update `premake4.lua` for C++ compilation
- ⬜ Create directory structure
- ⬜ Set up build system for C/C++ mix
- ⬜ Verify build works

#### StrategyContext Implementation
- ⬜ Create `include/StrategyContext.hpp`
- ⬜ Implement `src/StrategyContext.cpp`
- ⬜ Write unit tests for StrategyContext
- ⬜ Document StrategyContext API

#### IStrategy Interface
- ⬜ Create `include/IStrategy.hpp`
- ⬜ Write interface documentation
- ⬜ Create example stub implementation

### Week 2: Factory and Order Management

#### BaseStrategy Implementation
- ⬜ Create `include/BaseStrategy.hpp`
- ⬜ Implement `src/BaseStrategy.cpp`
- ⬜ Write unit tests
- ⬜ Create StrategyResult structure

#### StrategyFactory Implementation
- ⬜ Create `include/StrategyFactory.hpp`
- ⬜ Implement `src/StrategyFactory.cpp`
- ⬜ Register all 6 direct strategies (stubs) + AutoBBS dispatcher
- ⬜ Write unit tests

#### C API Wrapper
- ⬜ Create `src/AsirikuyStrategiesWrapper.cpp`
- ⬜ Implement `runStrategy()` wrapper
- ⬜ Add exception handling
- ⬜ Test C API wrapper

#### OrderBuilder Implementation
- ⬜ Create `include/OrderBuilder.hpp`
- ⬜ Implement `src/OrderBuilder.cpp`
- ⬜ Write unit tests
- ⬜ Test splitting logic

#### OrderManager Implementation
- ⬜ Create `include/OrderManager.hpp`
- ⬜ Implement `src/OrderManager.cpp`
- ⬜ Write unit tests

#### Indicators Wrapper
- ⬜ Create `include/Indicators.hpp`
- ⬜ Implement `src/Indicators.cpp`
- ⬜ Write unit tests

---

## Phase 2: Strategy Migration (Weeks 3-6)

### Week 3: Simple Strategies

#### RecordBarsStrategy
- ⬜ Create `include/strategies/RecordBarsStrategy.hpp`
- ⬜ Create `src/strategies/RecordBarsStrategy.cpp`
- ⬜ Migrate logic from `runRecordBars()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Side-by-side testing
- ⬜ Validate backtesting
- ⬜ Remove C implementation

#### TakeOverStrategy
- ⬜ Create `include/strategies/TakeOverStrategy.hpp`
- ⬜ Create `src/strategies/TakeOverStrategy.cpp`
- ⬜ Migrate logic from `runTakeOver()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

#### ScreeningStrategy
- ⬜ Create `include/strategies/ScreeningStrategy.hpp`
- ⬜ Create `src/strategies/ScreeningStrategy.cpp`
- ⬜ Migrate logic from `runScreening()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

#### TrendLimitStrategy
- ⬜ Create `include/strategies/TrendLimitStrategy.hpp`
- ⬜ Create `src/strategies/TrendLimitStrategy.cpp`
- ⬜ Migrate logic from `runTrendLimit()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

### Week 4-5: Trend Strategy Breakdown

#### Analysis
- ⬜ Analyze `TrendStrategy.c` (~9,286 lines, post-cleanup)
- ⬜ Identify all `workoutExecutionTrend_*` functions
- ⬜ Group related functions
- ⬜ Create breakdown document

#### MACD Strategies
- ⬜ Create `include/strategies/MACDDailyStrategy.hpp`
- ⬜ Create `include/strategies/MACDWeeklyStrategy.hpp`
- ⬜ Create `include/strategies/MACDDailyNewStrategy.hpp`
- ⬜ Extract common MACD logic
- ⬜ Implement all MACD strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C implementations

#### KeyK Strategy
- ⬜ Create `include/strategies/KeyKStrategy.hpp`
- ⬜ Create `src/strategies/KeyKStrategy.cpp`
- ⬜ Migrate `workoutExecutionTrend_KeyK()`
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### BBS Strategies
- ⬜ Create `include/strategies/BBSSwingStrategy.hpp`
- ⬜ Create `include/strategies/BBSBreakOutStrategy.hpp`
- ⬜ Extract common BBS logic
- ⬜ Implement all BBS strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Pivot Strategy
- ⬜ Create `include/strategies/PivotStrategy.hpp`
- ⬜ Create `src/strategies/PivotStrategy.cpp`
- ⬜ Migrate `workoutExecutionTrend_Pivot()`
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Limit Strategies
- ⬜ Create `include/strategies/LimitStrategy.hpp`
- ⬜ Create `include/strategies/LimitBBSStrategy.hpp`
- ⬜ Create `include/strategies/LimitBreakOutStrategy.hpp`
- ⬜ Extract common limit logic
- ⬜ Implement all limit strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Ichimoku Strategies
- ⬜ Create `include/strategies/IchimokuDailyV2Strategy.hpp`
- ⬜ Create `include/strategies/IchimokuDailyIndexStrategy.hpp`
- ⬜ Create `include/strategies/IchimokuDailyIndexV2Strategy.hpp`
- ⬜ Extract common Ichimoku logic
- ⬜ Implement all Ichimoku strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Order Splitting Migration
- ⬜ Identify all `splitBuyOrders_*` functions
- ⬜ Identify all `splitSellOrders_*` functions
- ⬜ Migrate to OrderBuilder patterns
- ⬜ Update all strategies to use OrderBuilder
- ⬜ Remove old splitting functions
- ⬜ Write tests for order splitting

### Week 6: Remaining Strategies

#### AutoBBS Strategy (Dispatcher)
- ⬜ Create `include/strategies/AutoBBSStrategy.hpp`
- ⬜ Create `src/strategies/AutoBBSStrategy.cpp`
- ⬜ Migrate `runAutoBBS()` dispatcher logic
- ⬜ Route to workoutExecutionTrend_* functions based on strategy_mode
- ⬜ Use factory to create sub-strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

**Note**: AutoBBS is a dispatcher that routes to ~30+ workoutExecutionTrend_* functions in TrendStrategy.c. These functions will be migrated separately.

---

## Phase 3: Cleanup (Week 7)

### Code Removal
- ⬜ Remove migrated strategy C files
- ⬜ Remove C strategy headers (if no longer needed)
- ⬜ Remove `TrendStrategy.c`
- ⬜ Remove `AsirikuyStrategies.c`
- ⬜ Clean up includes
- ⬜ Verify build after removals

### Code Optimization
- ⬜ Profile C++ code
- ⬜ Optimize hot paths
- ⬜ Optimize StrategyContext
- ⬜ Optimize Factory
- ⬜ Refactor duplicated code
- ⬜ Improve error handling
- ⬜ Improve const correctness
- ⬜ Check for memory leaks
- ⬜ Optimize allocations

### Documentation Updates
- ⬜ Update Doxygen comments
- ⬜ Generate API documentation
- ⬜ Update architecture diagrams
- ⬜ Update developer guide
- ⬜ Document migration decisions

---

## Phase 4: Testing (Week 8)

### Unit Testing
- ⬜ Run coverage analysis
- ⬜ Fill coverage gaps
- ⬜ Test StrategyContext
- ⬜ Test StrategyFactory
- ⬜ Test BaseStrategy
- ⬜ Test OrderBuilder
- ⬜ Test OrderManager
- ⬜ Test Indicators
- ⬜ Test each strategy class

### Integration Testing
- ⬜ Test C API wrapper
- ⬜ Test MQL integration
- ⬜ Test full strategy execution flow
- ⬜ Test order execution flow
- ⬜ Test multi-strategy execution

### Validation Testing
- ⬜ Run backtests for all strategies
- ⬜ Compare C vs C++ results
- ⬜ Benchmark each strategy
- ⬜ Compare performance with C version
- ⬜ Run memory leak detection
- ⬜ Test memory usage

### Final Verification
- ⬜ Test DLL loading
- ⬜ Test with MQL code
- ⬜ Test on different Windows versions
- ⬜ Run full test suite
- ⬜ Test edge cases
- ⬜ Verify API documentation
- ⬜ Verify architecture documentation
- ⬜ Verify developer guide

---

## Tracking

### Current Sprint
**Week**: [Current Week]  
**Focus**: [Current Phase]

### Progress Metrics
- **Total Tasks**: [Count]
- **Completed**: [Count]
- **In Progress**: [Count]
- **Pending**: [Count]
- **Blocked**: [Count]

### Blockers
- [List any blockers here]

### Notes
- [Add notes as needed]

---

**Document Version**: 1.0  
**Last Updated**: 2024  
**Status**: Active Tracking

