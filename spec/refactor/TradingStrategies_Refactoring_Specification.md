# TradingStrategies Module Refactoring Specification
## Option 1: C++ Internals with C API Wrapper (Recommended)

## Executive Summary

This document provides a detailed refactoring specification for the TradingStrategies module using **Option 1: C++ Internals with C API Wrapper**. This approach maintains 100% compatibility with the existing Windows DLL interface while modernizing the internal implementation with C++.

**Key Principle**: The DLL exports remain pure C with `__stdcall` convention, while the internal implementation uses modern C++ patterns.

**Current State (Post-Cleanup):**
- 6 direct trading strategies implemented in C (RECORD_BARS, TAKEOVER, SCREENING, AUTOBBS, AUTOBBSWEEKLY, TRENDLIMIT)
- AutoBBS dispatcher routes to ~30+ workoutExecutionTrend_* functions in TrendStrategy.c
- 10 source files in strategies directory, ~9,286 lines in TrendStrategy.c (reduced from 10,475)
- Function pointer-based strategy dispatch via switch statements
- DLL exports: `mql4_runStrategy`, `mql5_runStrategy`, `c_runStrategy`, `jf_runStrategy`
- Called from MetaTrader (MQL4/MQL5) via DLL
- **Cleanup completed**: Removed 20+ unused/outdated strategies (Kantu, KantuML, Coatl, Munay, AsirikuyBrain, etc.)
- **Modules removed**: AsirikuyMachineLearning, AsirikuyImageLib (dead code, unused)

**Target State:**
- C++ class-based architecture internally
- Pure C API wrapper for DLL exports (maintains compatibility)
- Strategy, Factory, and Template Method patterns
- Modular, testable, and maintainable codebase
- Zero breaking changes for MQL integration

---

## 1. Architecture Overview

### 1.1 Three-Layer Architecture

```
┌─────────────────────────────────────────────────────────┐
│ Layer 1: MQL/MetaTrader Interface (External)           │
│ - mql4_runStrategy()                                   │
│ - mql5_runStrategy()                                    │
│ - c_runStrategy()                                      │
│ - jf_runStrategy()                                     │
└────────────────────┬────────────────────────────────────┘
                     │ C API (__stdcall, extern "C")
┌────────────────────▼────────────────────────────────────┐
│ Layer 2: C API Wrapper (DLL Exports)                    │
│ - Pure C functions with __stdcall                       │
│ - Parameter conversion (MQL → StrategyParams)          │
│ - Exception handling wrapper                            │
│ - Return code conversion                                │
└────────────────────┬────────────────────────────────────┘
                     │ C++ API
┌────────────────────▼────────────────────────────────────┐
│ Layer 3: C++ Implementation (Internal)                 │
│ - IStrategy interface                                   │
│ - StrategyFactory                                       │
│ - Concrete strategy classes                            │
│ - Order management (Builder pattern)                   │
│ - Indicators wrapper                                    │
└─────────────────────────────────────────────────────────┘
```

### 1.2 DLL Export Structure

The DLL must maintain these exports (from `AsirikuyFrameworkAPI.def`):

```cpp
// Existing exports (MUST remain unchanged)
mql4_runStrategy
mql5_runStrategy
jf_runStrategy
c_runStrategy
```

**Critical Constraint**: These function signatures cannot change. The C++ refactoring is completely internal.

---

## 2. Detailed Implementation

### 2.1 C API Wrapper Layer

#### 2.1.1 Wrapper Functions

All DLL exports remain in C, but delegate to C++ internally:

```cpp
// File: src/AsirikuyStrategiesWrapper.cpp

#include "Precompiled.h"
#include "AsirikuyStrategies.h"
#include "StrategyContext.hpp"
#include "StrategyFactory.hpp"
#include <exception>

extern "C" {
    // Existing C function - delegates to C++ implementation
    AsirikuyReturnCode runStrategy(StrategyParams* pParams) {
        if (!pParams) {
            return NULL_POINTER;
        }
        
        try {
            // Convert C params to C++ context
            trading::StrategyContext ctx(pParams);
            
            // Use C++ factory to get strategy
            auto strategy = trading::StrategyFactory::create(ctx.getStrategyId());
            if (!strategy) {
                return INVALID_STRATEGY;
            }
            
            // Execute C++ strategy
            return strategy->execute(ctx);
            
        } catch (const std::bad_alloc& e) {
            pantheios_logprintf(PANTHEIOS_SEV_CRITICAL, 
                (PAN_CHAR_T*)"runStrategy() memory allocation failed: %s", e.what());
            return MEMORY_ALLOCATION_FAILED;
        } catch (const std::exception& e) {
            pantheios_logprintf(PANTHEIOS_SEV_CRITICAL, 
                (PAN_CHAR_T*)"runStrategy() exception: %s", e.what());
            return INTERNAL_ERROR;
        } catch (...) {
            pantheios_logputs(PANTHEIOS_SEV_CRITICAL, 
                (PAN_CHAR_T*)"runStrategy() unknown exception");
            return INTERNAL_ERROR;
        }
    }
    
    // Maintain existing getStrategyFunctions for backward compatibility
    AsirikuyReturnCode getStrategyFunctions(
        StrategyParams* pParams, 
        AsirikuyReturnCode(**runStrategyFunc)(StrategyParams*)) {
        
        if (!pParams || !runStrategyFunc) {
            return NULL_POINTER;
        }
        
        // Return pointer to wrapper function
        *runStrategyFunc = &runStrategy;
        return SUCCESS;
    }
}
```

#### 2.1.2 MQL-Specific Wrappers

The MQL wrappers in `AsirikuyFrameworkAPI` call `runStrategy()` internally, so no changes needed there. The flow is:

```
MQL → mql4_runStrategy() → [converts params] → runStrategy() → C++ implementation
```

### 2.2 C++ Core Implementation

#### 2.2.1 Strategy Context Wrapper

Wraps `StrategyParams` to provide C++-friendly interface:

```cpp
// File: include/StrategyContext.hpp

#pragma once

#include "AsirikuyDefines.h"
#include <string>
#include <memory>

namespace trading {

class Indicators;
class OrderManager;

class StrategyContext {
public:
    explicit StrategyContext(StrategyParams* params);
    
    // Non-copyable, movable
    StrategyContext(const StrategyContext&) = delete;
    StrategyContext& operator=(const StrategyContext&) = delete;
    StrategyContext(StrategyContext&&) = default;
    StrategyContext& operator=(StrategyContext&&) = default;
    
    // Accessors for StrategyParams
    StrategyId getStrategyId() const;
    const char* getSymbol() const;
    time_t getCurrentTime() const;
    double getSetting(SettingsIndex index) const;
    
    // Bid/Ask access
    const BidAsk& getBidAsk() const { return params_->bidAsk; }
    double getBid(int index = 0) const;
    double getAsk(int index = 0) const;
    
    // Account info
    const AccountInfo& getAccountInfo() const { return params_->accountInfo; }
    double getBalance() const;
    double getEquity() const;
    
    // Rates access
    RatesBuffers* getRatesBuffers() const { return params_->ratesBuffers; }
    const Rates& getRates(BaseRatesIndexes index) const;
    
    // Order info
    OrderInfo* getOrderInfo() const { return params_->orderInfo; }
    int getOrderCount() const;
    
    // Results
    StrategyResults* getResults() const { return params_->results; }
    
    // Indicators (lazy-loaded)
    Indicators& getIndicators();
    const Indicators& getIndicators() const;
    
    // Order management
    OrderManager& getOrderManager();
    
    // Raw access (for compatibility during migration)
    StrategyParams* getRawParams() const { return params_; }
    
private:
    StrategyParams* params_;  // Owned by caller, not this class
    mutable std::unique_ptr<Indicators> indicators_;
    mutable std::unique_ptr<OrderManager> orderManager_;
    
    void ensureIndicatorsLoaded() const;
    void ensureOrderManagerCreated() const;
};

} // namespace trading
```

**Implementation**:

```cpp
// File: src/StrategyContext.cpp

#include "StrategyContext.hpp"
#include "Indicators.hpp"
#include "OrderManager.hpp"

namespace trading {

StrategyContext::StrategyContext(StrategyParams* params) 
    : params_(params) {
    if (!params_) {
        throw std::invalid_argument("StrategyParams cannot be null");
    }
}

StrategyId StrategyContext::getStrategyId() const {
    return static_cast<StrategyId>(
        params_->settings[INTERNAL_STRATEGY_ID]);
}

const char* StrategyContext::getSymbol() const {
    return params_->tradeSymbol;
}

time_t StrategyContext::getCurrentTime() const {
    return params_->currentBrokerTime;
}

double StrategyContext::getSetting(SettingsIndex index) const {
    return params_->settings[index];
}

double StrategyContext::getBid(int index) const {
    if (index < 0 || index >= params_->bidAsk.arraySize) {
        throw std::out_of_range("Bid index out of range");
    }
    return params_->bidAsk.bid[index];
}

double StrategyContext::getAsk(int index) const {
    if (index < 0 || index >= params_->bidAsk.arraySize) {
        throw std::out_of_range("Ask index out of range");
    }
    return params_->bidAsk.ask[index];
}

Indicators& StrategyContext::getIndicators() {
    ensureIndicatorsLoaded();
    return *indicators_;
}

const Indicators& StrategyContext::getIndicators() const {
    ensureIndicatorsLoaded();
    return *indicators_;
}

void StrategyContext::ensureIndicatorsLoaded() const {
    if (!indicators_) {
        indicators_ = std::make_unique<Indicators>(params_);
    }
}

OrderManager& StrategyContext::getOrderManager() {
    ensureOrderManagerCreated();
    return *orderManager_;
}

void StrategyContext::ensureOrderManagerCreated() const {
    if (!orderManager_) {
        orderManager_ = std::make_unique<OrderManager>(params_);
    }
}

} // namespace trading
```

#### 2.2.2 Strategy Interface

```cpp
// File: include/IStrategy.hpp

#pragma once

#include "AsirikuyDefines.h"
#include <string>

namespace trading {

class StrategyContext;

class IStrategy {
public:
    virtual ~IStrategy() = default;
    
    // Execute the strategy
    virtual AsirikuyReturnCode execute(const StrategyContext& context) = 0;
    
    // Strategy metadata
    virtual std::string getName() const = 0;
    virtual StrategyId getId() const = 0;
    
    // Validation
    virtual bool validate(const StrategyContext& context) const = 0;
    
protected:
    // Helper to convert return codes
    static AsirikuyReturnCode toReturnCode(bool success) {
        return success ? SUCCESS : INTERNAL_ERROR;
    }
};

} // namespace trading
```

#### 2.2.3 Base Strategy (Template Method Pattern)

```cpp
// File: include/BaseStrategy.hpp

#pragma once

#include "IStrategy.hpp"
#include "StrategyContext.hpp"
#include "Indicators.hpp"
#include <memory>

namespace trading {

class BaseStrategy : public IStrategy {
public:
    explicit BaseStrategy(StrategyId id) : id_(id) {}
    
    // Template method - defines algorithm skeleton
    AsirikuyReturnCode execute(const StrategyContext& context) override final {
        // 1. Validate
        if (!validate(context)) {
            return INVALID_PARAMETERS;
        }
        
        // 2. Load indicators
        auto indicators = loadIndicators(context);
        if (!indicators) {
            return FAILED_TO_LOAD_INDICATORS;
        }
        
        // 3. Execute strategy-specific logic
        auto result = executeStrategy(context, *indicators);
        
        // 4. Update results
        updateResults(context, result);
        
        return result.code;
    }
    
    StrategyId getId() const override { return id_; }
    
protected:
    // Strategy-specific steps (to be overridden)
    virtual std::unique_ptr<Indicators> loadIndicators(
        const StrategyContext& ctx) = 0;
    
    virtual StrategyResult executeStrategy(
        const StrategyContext& ctx,
        const Indicators& indicators) = 0;
    
    virtual void updateResults(
        const StrategyContext& ctx,
        const StrategyResult& result) = 0;
    
    // Default validation (can be overridden)
    bool validate(const StrategyContext& context) const override {
        // Basic validation
        if (!context.getSymbol() || strlen(context.getSymbol()) == 0) {
            return false;
        }
        if (context.getBid(0) < EPSILON || context.getAsk(0) < EPSILON) {
            return false;
        }
        return true;
    }
    
private:
    StrategyId id_;
};

// Strategy result structure
struct StrategyResult {
    AsirikuyReturnCode code;
    std::vector<Order> orders;  // Generated orders
    std::string message;        // Optional message
};

} // namespace trading
```

#### 2.2.4 Strategy Factory

```cpp
// File: include/StrategyFactory.hpp

#pragma once

#include "IStrategy.hpp"
#include "AsirikuyDefines.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <vector>

namespace trading {

class StrategyFactory {
public:
    using StrategyCreator = std::function<std::unique_ptr<IStrategy>()>;
    
    // Create strategy by ID
    static std::unique_ptr<IStrategy> create(StrategyId id);
    
    // Create strategy from context
    static std::unique_ptr<IStrategy> create(const StrategyContext& context);
    
    // Get all available strategy IDs
    static std::vector<StrategyId> getAvailableStrategies();
    
    // Register a strategy (for extensibility)
    static void registerStrategy(StrategyId id, StrategyCreator creator);
    
private:
    static std::unordered_map<StrategyId, StrategyCreator>& getRegistry();
    
    static void initializeRegistry();
};

} // namespace trading
```

**Implementation**:

```cpp
// File: src/StrategyFactory.cpp

#include "StrategyFactory.hpp"
#include "strategies/AtipaqStrategy.hpp"
#include "strategies/AyotlStrategy.hpp"
#include "strategies/MACDDailyStrategy.hpp"
// ... include all strategy headers

namespace trading {

std::unique_ptr<IStrategy> StrategyFactory::create(StrategyId id) {
    initializeRegistry();
    
    auto& registry = getRegistry();
    auto it = registry.find(id);
    
    if (it != registry.end()) {
        return it->second();
    }
    
    return nullptr;
}

std::unique_ptr<IStrategy> StrategyFactory::create(const StrategyContext& context) {
    return create(context.getStrategyId());
}

void StrategyFactory::initializeRegistry() {
    static bool initialized = false;
    if (initialized) return;
    
    auto& registry = getRegistry();
    
    // Register all strategies
    registry[WATUKUSHAY_FE_BB] = []() { 
        return std::make_unique<WatukushayFE_BBStrategy>(); 
    };
    registry[ATIPAQ] = []() { 
        return std::make_unique<AtipaqStrategy>(); 
    };
    registry[AYOTL] = []() { 
        return std::make_unique<AyotlStrategy>(); 
    };
    registry[MACD_DAILY] = []() { 
        return std::make_unique<MACDDailyStrategy>(); 
    };
    // ... register all 6 direct strategies + AutoBBS dispatcher
    
    initialized = true;
}

std::unordered_map<StrategyId, StrategyFactory::StrategyCreator>& 
StrategyFactory::getRegistry() {
    static std::unordered_map<StrategyId, StrategyCreator> registry;
    return registry;
}

std::vector<StrategyId> StrategyFactory::getAvailableStrategies() {
    initializeRegistry();
    
    std::vector<StrategyId> strategies;
    for (const auto& pair : getRegistry()) {
        strategies.push_back(pair.first);
    }
    return strategies;
}

void StrategyFactory::registerStrategy(StrategyId id, StrategyCreator creator) {
    getRegistry()[id] = creator;
}

} // namespace trading
```

#### 2.2.5 Indicators Wrapper

```cpp
// File: include/Indicators.hpp

#pragma once

#include "Base.h"
#include "StrategyContext.hpp"
#include <memory>

namespace trading {

class Indicators {
public:
    explicit Indicators(StrategyParams* params);
    
    // Access Base_Indicators
    const Base_Indicators& getBase() const { return base_; }
    Base_Indicators& getBase() { return base_; }
    
    // Access strategy-specific Indicators struct
    const ::Indicators& getStrategy() const { return strategy_; }
    ::Indicators& getStrategy() { return strategy_; }
    
    // Convenience methods for common indicators
    Trend getDailyTrend() const;
    TrendPhase getDailyTrendPhase() const;
    double getDailyATR() const;
    double getDailyPivot() const;
    double getDailyS1() const;
    double getDailyR1() const;
    
    // Load indicators
    bool loadDailyIndicators(const StrategyContext& ctx);
    bool loadWeeklyIndicators(const StrategyContext& ctx);
    bool loadBaseIndicators(const StrategyContext& ctx);
    
private:
    Base_Indicators base_;
    ::Indicators strategy_;
    StrategyParams* params_;
    
    void initializeBaseIndicators();
};

} // namespace trading
```

#### 2.2.6 Order Management with Builder Pattern

```cpp
// File: include/OrderBuilder.hpp

#pragma once

#include "AsirikuyDefines.h"
#include <vector>
#include <memory>

namespace trading {

class Order {
public:
    OrderType type;
    double price;
    double stopLoss;
    double takeProfit;
    double lots;
    time_t expiration;
    
    AsirikuyReturnCode execute(StrategyParams* params) const;
};

class OrderBuilder {
public:
    OrderBuilder();
    
    // Fluent interface
    OrderBuilder& setType(OrderType type);
    OrderBuilder& setPrice(double price);
    OrderBuilder& setStopLoss(double sl);
    OrderBuilder& setTakeProfit(double tp);
    OrderBuilder& setLots(double lots);
    OrderBuilder& setExpiration(time_t expiration);
    
    // Split options
    OrderBuilder& splitIntoMultiple(size_t count);
    OrderBuilder& splitByATR(double atrMultiplier);
    OrderBuilder& splitByRisk(double riskPercent);
    
    // Build
    std::vector<Order> build() const;
    
private:
    OrderType type_;
    double price_;
    double stopLoss_;
    double takeProfit_;
    double lots_;
    time_t expiration_;
    
    size_t splitCount_;
    enum SplitMode { NONE, COUNT, ATR, RISK } splitMode_;
    double splitParam_;
    
    std::vector<Order> buildSplitOrders() const;
};

} // namespace trading
```

### 2.3 Example Strategy Implementation

#### 2.3.1 Simple Strategy: Atipaq

```cpp
// File: include/strategies/AtipaqStrategy.hpp

#pragma once

#include "BaseStrategy.hpp"

namespace trading {

class AtipaqStrategy : public BaseStrategy {
public:
    AtipaqStrategy() : BaseStrategy(ATIPAQ) {}
    
    std::string getName() const override { return "Atipaq"; }
    
protected:
    std::unique_ptr<Indicators> loadIndicators(
        const StrategyContext& ctx) override;
    
    StrategyResult executeStrategy(
        const StrategyContext& ctx,
        const Indicators& indicators) override;
    
    void updateResults(
        const StrategyContext& ctx,
        const StrategyResult& result) override;
};

} // namespace trading
```

```cpp
// File: src/strategies/AtipaqStrategy.cpp

#include "AtipaqStrategy.hpp"
#include "OrderBuilder.hpp"
#include "OrderManager.hpp"

namespace trading {

std::unique_ptr<Indicators> AtipaqStrategy::loadIndicators(
    const StrategyContext& ctx) {
    
    auto indicators = std::make_unique<Indicators>(ctx.getRawParams());
    if (!indicators->loadBaseIndicators(ctx)) {
        return nullptr;
    }
    return indicators;
}

StrategyResult AtipaqStrategy::executeStrategy(
    const StrategyContext& ctx,
    const Indicators& indicators) {
    
    StrategyResult result;
    
    // Original Atipaq logic here (migrated from C)
    // ... strategy-specific implementation ...
    
    // Build orders using builder pattern
    OrderBuilder builder;
    builder.setType(BUY)
           .setPrice(ctx.getBid(0))
           .setStopLoss(calculateStopLoss(ctx, indicators))
           .setTakeProfit(calculateTakeProfit(ctx, indicators))
           .setLots(calculateLots(ctx, indicators));
    
    result.orders = builder.build();
    result.code = SUCCESS;
    
    return result;
}

void AtipaqStrategy::updateResults(
    const StrategyContext& ctx,
    const StrategyResult& result) {
    
    // Update StrategyResults array in params
    // ... implementation ...
}

} // namespace trading
```

#### 2.3.2 Complex Strategy: MACD Daily (from TrendStrategy.c)

```cpp
// File: include/strategies/MACDDailyStrategy.hpp

#pragma once

#include "BaseStrategy.hpp"

namespace trading {

class MACDDailyStrategy : public BaseStrategy {
public:
    MACDDailyStrategy() : BaseStrategy(AUTOBBS) {}  // Uses AUTOBBS ID with mode
    
    std::string getName() const override { return "MACD Daily"; }
    
protected:
    std::unique_ptr<Indicators> loadIndicators(
        const StrategyContext& ctx) override;
    
    StrategyResult executeStrategy(
        const StrategyContext& ctx,
        const Indicators& indicators) override;
    
    void updateResults(
        const StrategyContext& ctx,
        const StrategyResult& result) override;
    
private:
    // Helper methods (migrated from workoutExecutionTrend_MACD_Daily)
    double calculateStopLoss(const StrategyContext& ctx, 
                           const Indicators& indicators);
    double calculateTakeProfit(const StrategyContext& ctx,
                              const Indicators& indicators);
    bool shouldEnterLong(const StrategyContext& ctx,
                        const Indicators& indicators);
    bool shouldEnterShort(const StrategyContext& ctx,
                         const Indicators& indicators);
};

} // namespace trading
```

---

## 3. Migration Strategy

### 3.1 Phase 1: Foundation (Weeks 1-2)

#### Week 1: Core Infrastructure
- [ ] Create C++ namespace structure (`trading::`)
- [ ] Implement `StrategyContext` class
- [ ] Implement `IStrategy` interface
- [ ] Create `BaseStrategy` abstract class
- [ ] Set up C++ build configuration (update premake4.lua)
- [ ] Create C API wrapper (`runStrategy` delegates to C++)

#### Week 2: Factory and Order Management
- [ ] Implement `StrategyFactory` with registry
- [ ] Create `OrderBuilder` class
- [ ] Implement `OrderManager` class
- [ ] Create `Indicators` wrapper class
- [ ] Unit tests for core components

**Deliverables:**
- Core C++ infrastructure in place
- C API wrapper working
- Factory can create strategies
- Order builder functional

### 3.2 Phase 2: Strategy Migration (Weeks 3-6)

#### Week 3: Simple Strategies
Migrate 3-4 simple strategies to establish pattern:
- [ ] AtipaqStrategy
- [ ] AyotlStrategy
- [ ] CoatlStrategy
- [ ] TestEAStrategy

**Approach:**
1. Create C++ class for strategy
2. Migrate logic from C function
3. Test side-by-side (C vs C++)
4. Verify identical behavior
5. Switch factory to use C++ version

#### Week 4-5: Trend Strategy Breakdown
Break down `TrendStrategy.c` (~9,286 lines, post-cleanup) into classes:

- [ ] Identify all `workoutExecutionTrend_*` functions
- [ ] Group related functions:
  - MACD strategies → `MACDDailyStrategy`, `MACDWeeklyStrategy`
  - KeyK strategies → `KeyKStrategy`
  - BBS strategies → `BBSSwingStrategy`, `BBSBreakOutStrategy`
  - Pivot strategies → `PivotStrategy`
  - Limit strategies → `LimitStrategy`
  - Ichimoku strategies → `IchimokuDailyStrategy`
- [ ] Extract order splitting functions to `OrderBuilder`
- [ ] Create shared base class for common trend logic

#### Week 6: Remaining Strategies
- [ ] Migrate remaining simple strategies
- [ ] Migrate complex strategies (Swing, Grid, etc.)
- [ ] Update AutoBBS dispatcher to use C++ strategies

**Deliverables:**
- All strategies migrated to C++
- Monolithic files broken down
- Comprehensive unit tests

### 3.3 Phase 3: Cleanup and Optimization (Week 7)

- [ ] Remove old C strategy implementations
- [ ] Optimize C++ code
- [ ] Refactor common patterns
- [ ] Update documentation
- [ ] Performance profiling and optimization

### 3.4 Phase 4: Testing and Validation (Week 8)

- [ ] Unit tests for all strategies
- [ ] Integration tests
- [ ] Backtesting validation (compare C vs C++ results)
- [ ] Performance benchmarks
- [ ] DLL compatibility testing with MQL
- [ ] Memory leak detection

---

## 4. DLL Compatibility Requirements

### 4.1 Export Stability

**Critical**: These exports MUST remain unchanged:

```cpp
// From AsirikuyFrameworkAPI.def
mql4_runStrategy
mql5_runStrategy
jf_runStrategy
c_runStrategy
```

The function signatures in `AsirikuyFrameworkAPI` call `runStrategy()` internally, which we wrap. No changes needed to MQL interface layer.

### 4.2 Calling Convention

All DLL exports use `__stdcall`:

```cpp
extern "C" {
    int __stdcall mql4_runStrategy(...) {
        // Converts MQL params to StrategyParams
        // Calls runStrategy()
    }
}
```

### 4.3 Exception Handling

All C++ exceptions must be caught at the C API boundary:

```cpp
extern "C" {
    AsirikuyReturnCode runStrategy(StrategyParams* pParams) {
        try {
            // C++ code
        } catch (...) {
            return INTERNAL_ERROR;
        }
    }
}
```

### 4.4 Memory Management

- C++ uses RAII (automatic memory management)
- No new allocations that need to be freed by caller
- StrategyParams is owned by caller (MQL)
- C++ classes hold smart pointers to internal data

### 4.5 ABI Stability

- C API functions maintain stable signatures
- Internal C++ implementation can change freely
- StrategyParams structure remains unchanged (owned by framework)

---

## 5. Testing Strategy

### 5.1 Side-by-Side Testing

During migration, run both C and C++ versions:

```cpp
// Test harness
void testStrategy(StrategyId id, StrategyParams* params) {
    // Run C version
    AsirikuyReturnCode cResult = runStrategy_C_Impl(params);
    
    // Run C++ version
    AsirikuyReturnCode cppResult = runStrategy(params);
    
    // Compare results
    assert(cResult == cppResult);
    assert(memcmp(params->results, expectedResults, ...) == 0);
}
```

### 5.2 Backtesting Validation

Run full backtests with both implementations and compare:
- Trade entries/exits
- Order sizes
- Stop loss/take profit levels
- Equity curves
- Performance metrics

### 5.3 DLL Integration Testing

Test with actual MQL code:
- Load DLL in MetaTrader
- Run strategies
- Verify no crashes
- Verify correct behavior

---

## 6. Build Configuration

### 6.1 Premake4 Updates

Update `dev/TradingStrategies/premake4.lua`:

```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"  -- Changed from "C"
  cppdialect "C++17"
  
  files{
    "**.h", 
    "**.hpp",
    "**.c", 
    "**.cpp",
    "**.hpp"
  }
  
  -- C++ specific settings
  flags { "C++17" }
  
  -- Keep C compatibility
  defines { "C_COMPATIBLE_API" }
```

### 6.2 Include Paths

Add C++ include directories:
- `include/` (for .hpp files)
- Keep existing C includes for compatibility

---

## 7. Code Organization

### 7.1 Directory Structure

```
TradingStrategies/
├── include/
│   ├── AsirikuyStrategies.h          # C API (unchanged)
│   ├── StrategyContext.hpp           # C++ context wrapper
│   ├── IStrategy.hpp                 # Strategy interface
│   ├── BaseStrategy.hpp              # Base class
│   ├── StrategyFactory.hpp           # Factory
│   ├── Indicators.hpp                # Indicators wrapper
│   ├── OrderBuilder.hpp              # Order builder
│   ├── OrderManager.hpp              # Order management
│   └── strategies/
│       ├── AtipaqStrategy.hpp
│       ├── MACDDailyStrategy.hpp
│       └── ...
├── src/
│   ├── AsirikuyStrategies.c          # C implementation (legacy, to be removed)
│   ├── AsirikuyStrategiesWrapper.cpp # C API wrapper
│   ├── StrategyContext.cpp
│   ├── StrategyFactory.cpp
│   ├── Indicators.cpp
│   ├── OrderBuilder.cpp
│   └── strategies/
│       ├── AtipaqStrategy.cpp
│       ├── MACDDailyStrategy.cpp
│       └── ...
└── tests/
    ├── StrategyFactoryTest.cpp
    ├── AtipaqStrategyTest.cpp
    └── ...
```

### 7.2 File Naming Convention

- C++ headers: `.hpp`
- C++ sources: `.cpp`
- C headers: `.h` (for compatibility)
- C sources: `.c` (legacy, to be removed)

---

## 8. Risk Mitigation

### 8.1 Incremental Migration

- Migrate one strategy at a time
- Test each migration thoroughly
- Keep C code until C++ version validated
- Use feature flags if needed

### 8.2 Rollback Plan

- Keep C implementations until migration complete
- Use factory to switch between C and C++:
  ```cpp
  if (USE_CPP_STRATEGIES) {
      return createCppStrategy(id);
  } else {
      return createCStrategy(id);  // Legacy
  }
  ```

### 8.3 Validation

- Automated tests compare C vs C++ outputs
- Backtesting validation
- Performance benchmarks
- Memory leak detection

---

## 9. Success Criteria

### 9.1 Functional Requirements
- [ ] All 6 direct strategies + AutoBBS dispatcher + ~30+ workoutExecutionTrend_* functions work identically to C version
- [ ] Backtesting results match original (within rounding)
- [ ] DLL exports unchanged
- [ ] MQL integration works without modification
- [ ] No memory leaks
- [ ] Performance within 5% of original

### 9.2 Code Quality
- [ ] All strategies in C++ classes
- [ ] No code duplication
- [ ] 90%+ test coverage
- [ ] All linter warnings resolved
- [ ] Largest file < 500 lines

### 9.3 Maintainability
- [ ] New strategy can be added in < 1 day
- [ ] Clear architecture documentation
- [ ] Easy onboarding for new developers
- [ ] Self-documenting code

---

## 10. Timeline Summary

| Phase | Duration | Key Deliverables |
|-------|----------|------------------|
| Phase 1: Foundation | 2 weeks | Core C++ infrastructure, C API wrapper, Factory |
| Phase 2: Migration | 4 weeks | All strategies migrated, monolithic files broken down |
| Phase 3: Cleanup | 1 week | Remove C code, optimize, document |
| Phase 4: Testing | 1 week | Comprehensive testing, validation |
| **Total** | **8 weeks** | **Complete refactored module** |

---

## 11. Next Steps

1. **Review & Approval**: Get stakeholder approval
2. **Setup Branch**: Create `refactor/cpp-migration` branch
3. **Build Configuration**: Update premake4.lua for C++
4. **Phase 1 Start**: Implement core infrastructure
5. **Weekly Reviews**: Progress reviews and adjustments

---

## Appendix

### A.1. Strategy ID Mapping

Current strategy IDs (from `AsirikuyStrategies.c` - post-cleanup):
- RECORD_BARS = 19
- TAKEOVER = 26
- SCREENING = 27
- AUTOBBS = 29
- AUTOBBSWEEKLY = 30
- TRENDLIMIT = 31

**Note**: AutoBBS (ID 29) is a dispatcher that routes to ~30+ workoutExecutionTrend_* functions in TrendStrategy.c based on strategy_mode parameter.

**Removed strategies** (cleaned up before refactoring):
- WATUKUSHAY_FE_BB/CCI/RSI, ATIPAQ, AYOTL, COATL, COMITL variants, GODS_GIFT_ATR, QALLARYI, QUIMICHI, SAPAQ, ASIRIKUY_BRAIN, TEYACANANI, RUPHAY, TEST_EA, EURCHF_GRID, KANTU, KANTU_ML, KELPIE, KEYK, BUDAN, MUNAY, RENKO_TEST

These IDs remain unchanged. The C++ factory uses the same IDs.

### A.2. Key C Structures (Unchanged)

- `StrategyParams` - Owned by framework, passed to strategies
- `Base_Indicators` - Wrapped by C++ `Indicators` class
- `Indicators` - Wrapped by C++ `Indicators` class
- `OrderInfo` - Used by `OrderManager`
- `StrategyResults` - Updated by strategies

### A.3. Migration Checklist Template

For each strategy:
- [ ] Create C++ class header
- [ ] Create C++ class implementation
- [ ] Migrate logic from C function
- [ ] Register in StrategyFactory
- [ ] Write unit tests
- [ ] Test side-by-side with C version
- [ ] Validate backtesting results
- [ ] Remove C implementation
- [ ] Update documentation

---

**Document Version**: 2.0  
**Last Updated**: 2024  
**Status**: Detailed Specification - Option 1 (C++ Internals + C API)  
**Target**: Windows DLL for MetaTrader/MQL Integration
