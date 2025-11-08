# Phase 2: Strategy Migration - Detailed Plan

## Overview

**Duration**: 4 weeks  
**Goal**: Migrate all 33+ strategies from C to C++ classes

**Approach**: Incremental migration, one strategy at a time, with side-by-side testing

---

## Week 3: Simple Strategies (Foundation)

### Strategy: AtipaqStrategy

#### Tasks
- [ ] Create `include/strategies/AtipaqStrategy.hpp`
  - [ ] Inherit from BaseStrategy
  - [ ] Define class structure
- [ ] Create `src/strategies/AtipaqStrategy.cpp`
  - [ ] Migrate logic from `runAtipaq()` in `Atipaq.c`
  - [ ] Implement `loadIndicators()`
  - [ ] Implement `executeStrategy()`
  - [ ] Implement `updateResults()`
  - [ ] Use OrderBuilder for orders
- [ ] Register in StrategyFactory
- [ ] Write unit tests
- [ ] Side-by-side testing (C vs C++)
- [ ] Validate backtesting results match
- [ ] Remove C implementation (or mark as deprecated)

#### Migration Checklist
- [ ] Logic migrated correctly
- [ ] Order generation matches C version
- [ ] Results update correctly
- [ ] Error handling works
- [ ] Performance acceptable

#### Deliverables
- AtipaqStrategy class implemented
- Tests passing
- Validated against C version

---

### Strategy: AyotlStrategy

#### Tasks
- [ ] Create `include/strategies/AyotlStrategy.hpp`
- [ ] Create `src/strategies/AyotlStrategy.cpp`
  - [ ] Migrate from `runAyotl()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- AyotlStrategy implemented and validated

---

### Strategy: CoatlStrategy

#### Tasks
- [ ] Create `include/strategies/CoatlStrategy.hpp`
- [ ] Create `src/strategies/CoatlStrategy.cpp`
  - [ ] Migrate from `runCoatl()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- CoatlStrategy implemented and validated

---

### Strategy: TestEAStrategy

#### Tasks
- [ ] Create `include/strategies/TestEAStrategy.hpp`
- [ ] Create `src/strategies/TestEAStrategy.cpp`
  - [ ] Migrate from `runTestEA()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- TestEAStrategy implemented and validated

---

### Week 3 Completion

- [ ] 4 simple strategies migrated
- [ ] All tests passing
- [ ] All validations complete
- [ ] Pattern established for remaining strategies

---

## Week 4-5: Trend Strategy Breakdown

### Analysis Phase

#### Tasks
- [ ] Analyze `TrendStrategy.c` (10,475 lines)
- [ ] Identify all `workoutExecutionTrend_*` functions
- [ ] Group related functions:
  - [ ] MACD functions → MACD strategies
  - [ ] KeyK functions → KeyK strategies
  - [ ] BBS functions → BBS strategies
  - [ ] Pivot functions → Pivot strategies
  - [ ] Limit functions → Limit strategies
  - [ ] Ichimoku functions → Ichimoku strategies
- [ ] Identify order splitting functions
- [ ] Create breakdown document

#### Deliverables
- Strategy breakdown document
- Function grouping list
- Migration plan for TrendStrategy.c

---

### MACD Strategies

#### Tasks
- [ ] Create `include/strategies/MACDDailyStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Daily()`
- [ ] Create `include/strategies/MACDWeeklyStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Weekly()`
- [ ] Create `include/strategies/MACDDailyNewStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Daily_New()`
- [ ] Extract common MACD logic to base class
- [ ] Implement all MACD strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate each strategy
- [ ] Remove C implementations

#### Deliverables
- All MACD strategies migrated
- Common logic extracted
- Tests and validation complete

---

### KeyK Strategy

#### Tasks
- [ ] Create `include/strategies/KeyKStrategy.hpp`
- [ ] Create `src/strategies/KeyKStrategy.cpp`
  - [ ] Migrate `workoutExecutionTrend_KeyK()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- KeyKStrategy implemented and validated

---

### BBS Strategies

#### Tasks
- [ ] Create `include/strategies/BBSSwingStrategy.hpp`
  - [ ] Migrate BBS swing logic
- [ ] Create `include/strategies/BBSBreakOutStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_BBS_BreakOut()`
- [ ] Extract common BBS logic
- [ ] Implement all BBS strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All BBS strategies migrated
- Common logic extracted

---

### Pivot Strategy

#### Tasks
- [ ] Create `include/strategies/PivotStrategy.hpp`
- [ ] Create `src/strategies/PivotStrategy.cpp`
  - [ ] Migrate `workoutExecutionTrend_Pivot()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- PivotStrategy implemented and validated

---

### Limit Strategies

#### Tasks
- [ ] Create `include/strategies/LimitStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit()`
- [ ] Create `include/strategies/LimitBBSStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit_BBS()`
- [ ] Create `include/strategies/LimitBreakOutStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit_BreakOutOnPivot()`
- [ ] Extract common limit logic
- [ ] Implement all limit strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All limit strategies migrated

---

### Ichimoku Strategies

#### Tasks
- [ ] Create `include/strategies/IchimokuDailyV2Strategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_V2()`
- [ ] Create `include/strategies/IchimokuDailyIndexStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_Index()`
- [ ] Create `include/strategies/IchimokuDailyIndexV2Strategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_Index_V2()`
- [ ] Extract common Ichimoku logic
- [ ] Implement all Ichimoku strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All Ichimoku strategies migrated

---

### Order Splitting Functions Migration

#### Tasks
- [ ] Identify all `splitBuyOrders_*` and `splitSellOrders_*` functions
- [ ] Migrate to OrderBuilder patterns
- [ ] Update all strategies to use OrderBuilder
- [ ] Remove old splitting functions
- [ ] Write tests for order splitting

#### Deliverables
- Order splitting migrated to OrderBuilder
- All strategies using new pattern

---

## Week 6: Remaining Strategies

### AutoBBS Strategy (Dispatcher)

#### Tasks
- [ ] Create `include/strategies/AutoBBSStrategy.hpp`
- [ ] Create `src/strategies/AutoBBSStrategy.cpp`
  - [ ] Migrate `runAutoBBS()` dispatcher logic
  - [ ] Use factory to create sub-strategies
  - [ ] Route based on strategy_mode
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- AutoBBSStrategy implemented as dispatcher

---

### Remaining Simple Strategies

Migrate remaining strategies following established pattern:

- [ ] WatukushayFE_BBStrategy
- [ ] WatukushayFE_CCIStrategy
- [ ] WatukushayFE_RSIStrategy
- [ ] Comitl_BBStrategy
- [ ] Comitl_KCStrategy
- [ ] Comitl_PAStrategy
- [ ] GodsGiftATRStrategy
- [ ] QallaryiStrategy
- [ ] QuimichiStrategy
- [ ] SapaqStrategy
- [ ] AsirikuyBrainStrategy
- [ ] TeyacananiStrategy
- [ ] RuphayStrategy
- [ ] EURCHF_gridStrategy
- [ ] KantuStrategy
- [ ] RecordBarsStrategy
- [ ] MunayStrategy
- [ ] RenkoTestStrategy
- [ ] KantuMLStrategy
- [ ] KelpieStrategy
- [ ] BBSStrategy
- [ ] TakeOverStrategy
- [ ] ScreeningStrategy
- [ ] TrendLimitStrategy
- [ ] BuDanStrategy

#### Pattern for Each
1. Create header and source files
2. Migrate logic from C function
3. Register in factory
4. Write tests
5. Validate
6. Remove C code

---

## Migration Template

For each strategy, follow this template:

### 1. Analysis
- [ ] Read C implementation
- [ ] Identify dependencies
- [ ] Identify order splitting logic
- [ ] Identify indicator usage

### 2. Design
- [ ] Design class structure
- [ ] Identify base class (BaseStrategy or custom)
- [ ] Plan order building
- [ ] Plan indicator loading

### 3. Implementation
- [ ] Create header file
- [ ] Create source file
- [ ] Migrate logic
- [ ] Use OrderBuilder
- [ ] Use Indicators wrapper

### 4. Testing
- [ ] Write unit tests
- [ ] Side-by-side testing
- [ ] Backtesting validation
- [ ] Performance check

### 5. Integration
- [ ] Register in factory
- [ ] Update documentation
- [ ] Code review
- [ ] Remove C code

---

## Validation Strategy

### Side-by-Side Testing

For each strategy:
```cpp
void validateStrategy(StrategyId id, StrategyParams* params) {
    // Run C version
    StrategyParams cParams = *params;
    AsirikuyReturnCode cResult = runStrategy_C(id, &cParams);
    
    // Run C++ version
    StrategyParams cppParams = *params;
    AsirikuyReturnCode cppResult = runStrategy(&cppParams);
    
    // Compare
    assert(cResult == cppResult);
    assert(compareResults(&cParams.results, &cppParams.results));
}
```

### Backtesting Validation

- Run full backtest with C version
- Run full backtest with C++ version
- Compare:
  - Trade entries/exits
  - Order sizes
  - Stop loss/take profit levels
  - Equity curves
  - Performance metrics

---

## Phase 2 Completion Criteria

### Functional Requirements
- [ ] All 33+ strategies migrated to C++
- [ ] All strategies registered in factory
- [ ] TrendStrategy.c broken down into classes
- [ ] Order splitting migrated to OrderBuilder
- [ ] All strategies use Indicators wrapper

### Quality Requirements
- [ ] All unit tests passing
- [ ] All strategies validated against C versions
- [ ] Backtesting results match (within rounding)
- [ ] Performance within 5% of original
- [ ] Code reviewed

### Code Quality
- [ ] No code duplication
- [ ] Largest file < 500 lines
- [ ] Clear class hierarchy
- [ ] Well-documented

---

## Risks and Mitigation

### Risk: Large Strategy Files
- **Mitigation**: Break down into smaller classes, extract common logic

### Risk: Complex Logic Migration
- **Mitigation**: Migrate incrementally, test frequently, compare outputs

### Risk: Performance Regression
- **Mitigation**: Profile each strategy, optimize hot paths

### Risk: Missing Edge Cases
- **Mitigation**: Comprehensive testing, side-by-side validation

---

## Deliverables Summary

1. **Code**
   - All 33+ strategy classes implemented
   - TrendStrategy.c broken down
   - Order splitting migrated
   - All strategies registered

2. **Tests**
   - Unit tests for all strategies
   - Side-by-side validation tests
   - Backtesting validation

3. **Documentation**
   - Strategy migration log
   - Class documentation
   - Architecture updates

---

## Next Phase

After Phase 2 completion, proceed to:
- **[Phase 3: Cleanup](./PLAN_PHASE3_CLEANUP.md)**

---

**Document Version**: 1.0  
**Last Updated**: 2024  
**Status**: Ready for Implementation

