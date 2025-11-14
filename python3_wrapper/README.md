# Python 3 Wrapper for AsirikuyFrameworkAPI

⚠️ **STATUS: DEFERRED** (December 2024)

**Migration to AsirikuyFrameworkAPI has been deferred.** CTester will continue using CTesterFrameworkAPI directly, which already works perfectly. This wrapper is preserved for future reference if migration is reconsidered.

---

This package provides a Python 3 interface to the AsirikuyFrameworkAPI C library, specifically designed for CTester integration.

## Structure

```
python3_wrapper/
├── __init__.py              # Package initialization and exports
├── ctester_wrapper.py        # Main wrapper class and low-level API
├── structures.py             # ctypes structure definitions
├── converters.py             # Structure conversion functions
├── execution_engine.py        # Per-bar execution loop (TODO)
└── README.md                 # This file
```

## Modules

### `ctester_wrapper.py`
Main wrapper class (`AsirikuyFrameworkWrapper`) that:
- Loads the AsirikuyFrameworkAPI library
- Provides low-level API functions (`initInstanceC`, `deinitInstance`, `c_runStrategy`)
- Manages instance lifecycle
- Handles Python 3 string/bytes encoding

### `structures.py`
ctypes structure definitions matching `CTesterDefines.h`:
- `CRates`: Single OHLCV candle
- `COrderInfo`: Order/trade information
- `CRatesInfo`: Rate buffer metadata
- `StrategyResults`: Strategy execution results
- Error codes and constants

### `converters.py`
Functions to convert between CTester structures and AsirikuyFrameworkAPI structures:
- `convertRateToCRates()`: Convert Rate → CRates
- `convertOrderInfoToCOrderInfo()`: Convert OrderInfo → COrderInfo
- `convertRateInfoToCRatesInfo()`: Convert RateInfo → CRatesInfo
- And reverse conversions

### `execution_engine.py`
Per-bar execution engine (TODO):
- `runPortfolioTest()`: High-level batch API compatible with CTester
- Per-bar execution loop
- Order management
- Statistics accumulation
- Callback generation

## Usage

### Basic Usage

```python
from python3_wrapper import AsirikuyFrameworkWrapper

# Create wrapper (auto-detects library path)
wrapper = AsirikuyFrameworkWrapper()

# Initialize instance
result = wrapper.initInstanceC(
    instance_id=1,
    is_testing=1,  # 1 for backtesting
    config_path="./config/AsirikuyConfig.xml",
    account_name="TestAccount"
)

# Execute strategy for one bar
result = wrapper.c_runStrategy(
    settings=settings_array,
    symbol="EURUSD",
    account_currency="USD",
    broker_name="TestBroker",
    ref_broker_name="TestBroker",
    current_broker_time=byref(current_time),
    open_orders_count=byref(order_count),
    order_info=order_array,
    account_info=account_array,
    bid_ask=bid_ask_array,
    rates_info=rates_info_array,
    rates_array=rates_arrays,
    results=results_array
)

# Cleanup
wrapper.deinitInstance(instance_id=1)
```

### High-Level API (TODO)

```python
from python3_wrapper import runPortfolioTest

# Compatible with CTester's runPortfolioTest interface
result = runPortfolioTest(
    numSystemsInPortfolio=2,
    settings=settings_arrays,
    symbols=symbols,
    accountCurrency="USD",
    brokerName="TestBroker",
    refBrokerName="TestBroker",
    accountInfo=account_info_arrays,
    testSettings=test_settings,
    ratesInfoArray=rates_info_arrays,
    numCandles=1000,
    ratesArray=rates_arrays,
    minLotSize=0.01,
    testUpdate_callback=testUpdate,
    testFinished_callback=testFinished,
    signalUpdate_callback=signalUpdate
)
```

## Key Differences from CTesterFrameworkAPI

1. **Execution Model**: AsirikuyFrameworkAPI is per-bar, CTesterFrameworkAPI is batch
2. **Initialization**: Per-instance (`initInstanceC`) vs global (`initCTesterFramework`)
3. **Callbacks**: AsirikuyFrameworkAPI returns results directly, CTesterFrameworkAPI uses callbacks
4. **Structure Differences**: 
   - `Rate` includes swap data, `CRates` does not
   - Field name case differences (`requiredTimeFrame` vs `requiredTimeframe`)
   - Field name typo (`expiration` vs `expiriation`)

## Status

⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI has been deferred. CTester continues using CTesterFrameworkAPI directly.

- ✅ T025: Skeleton created (preserved for future use)
- ⚠️ T026-T028: DEFERRED
- ⚠️ Execution engine: DEFERRED

**Current Architecture**: CTester → CTesterFrameworkAPI (direct integration, no wrapper needed)

## References

- **CTesterDefines.h**: `dev/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h`
- **CTesterTradingStrategiesAPI.h**: `dev/AsirikuyFrameworkAPI/include/CTester/CTesterTradingStrategiesAPI.h`
- **Interface Documentation**: `dev/specs/001-trading-strategies-python-refactor/CTESTER_INTERFACE_DOCUMENTATION.md`

