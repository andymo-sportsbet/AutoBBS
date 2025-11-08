# TrendStrategy Support Documentation

## Overview

The `TrendStrategy` module is a comprehensive trading strategy system designed for the AutoBBS trading framework. It implements multiple trend-following and breakout strategies that adapt to different market conditions and timeframes. The system operates primarily on 15-minute timeframes with daily and weekly trend analysis.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Core Strategy Functions](#core-strategy-functions)
3. [Order Management Functions](#order-management-functions)
4. [Strategy Details](#strategy-details)
5. [Indicators and Parameters](#indicators-and-parameters)
6. [Risk Management](#risk-management)
7. [Entry and Exit Logic](#entry-and-exit-logic)
8. [Symbol-Specific Configurations](#symbol-specific-configurations)

---

## Architecture Overview

### Primary Timeframe
- **Primary Rates**: 15-minute bars (B_PRIMARY_RATES)
- **Secondary Rates**: 1-hour bars (B_SECONDARY_RATES)
- **Daily Rates**: Daily bars (B_DAILY_RATES)
- **Weekly Rates**: Weekly bars (B_WEEKLY_RATES)

### Strategy Execution Flow

The system uses a hierarchical approach where strategies are selected based on:
1. **Daily Trend Phase** - Determines which strategy to use
2. **Market Conditions** - Adapts to volatility and trend strength
3. **Symbol-Specific Rules** - Custom configurations per trading pair

### Trend Phases

The system recognizes several trend phases:
- `BEGINNING_UP_PHASE` / `BEGINNING_DOWN_PHASE` - Early trend formation
- `MIDDLE_UP_PHASE` / `MIDDLE_DOWN_PHASE` - Established trend continuation
- `MIDDLE_UP_RETREAT_PHASE` / `MIDDLE_DOWN_RETREAT_PHASE` - Pullback within trend
- `RANGE_PHASE` - Consolidation/no clear trend

---

## Core Strategy Functions

### 1. `workoutExecutionTrend_Auto`
**Purpose**: Main auto-strategy that routes to appropriate sub-strategies based on trend phase.

**Logic**:
- Checks for KeyK trigger (if enabled) - takes priority
- Routes to different strategies based on `dailyTrend_Phase`:
  - **Beginning Phase**: `DailyOpen` or `Pivot` strategies, with `BBS_BreakOut` as fallback
  - **Middle Phase**: Same as beginning, but allows more aggressive entries
  - **Retreat Phase**: `MIDDLE_RETREAT_PHASE` strategy
  - **Range Phase**: Exits all positions or adjusts stop losses

**Key Features**:
- Filters trades based on MA50 on daily chart
- Closes short-term orders if intraday ATR exceeds limits
- Implements profit management

### 2. `workoutExecutionTrend_KeyK`
**Purpose**: High-priority strategy triggered by key price levels.

**Entry Conditions**:
- Uses intraday high/low from key index
- Risk: 2% per trade
- Split mode: 9 (KeyK specific)
- No take profit (TP mode: 0)

**Logic**:
- Enters when price breaks key intraday levels
- Only trades in direction of intraday trend

### 3. `workoutExecutionTrend_BBS_BreakOut`
**Purpose**: Breakout strategy using BBS (Bollinger Band System) signals on 15M timeframe.

**Entry Conditions**:
- **Long**: 
  - Daily trend phase is UP (beginning or middle)
  - BBS trend execution signal is UP (1)
  - Price above daily MA50 baseline
  - Price above daily S1 support
  - BBS index matches current bar or price retreated within breakout range

- **Short**:
  - Daily trend phase is DOWN (beginning or middle)
  - BBS trend execution signal is DOWN (-1)
  - Price below daily MA50 baseline
  - Price below daily S1 resistance
  - BBS index matches current bar or price retreated within breakout range

**Risk Management**:
- Stop loss: Daily S/R or daily ATR
- Risk: 1% per trade
- Split mode: 3 (BBS Breakout)

### 4. `workoutExecutionTrend_Pivot`
**Purpose**: Entry strategy based on daily pivot points.

**Entry Conditions**:
- Uses daily pivot, S1, R1 levels
- Enters when price approaches pivot levels in trend direction
- Similar to BBS_BreakOut but uses pivot calculations

### 5. `workoutExecutionTrend_MIDDLE_RETREAT_PHASE`
**Purpose**: Catches pullbacks within established trends.

**Entry Conditions**:
- **Long**: 
  - Middle UP retreat phase
  - BBS trend execution is UP
  - Price retreated back within breakout bar range
  - Price within 66.6% of daily ATR from daily S
  - Price above daily S

- **Short**:
  - Middle DOWN retreat phase
  - BBS trend execution is DOWN
  - Price retreated back within breakout bar range
  - Price within 66.6% of daily ATR from daily S
  - Price below daily S

**Risk Management**:
- Risk: 1% per trade
- Split mode: 0 (standard)
- Sub-trade mode: 2 (retreat specific)

### 6. `workoutExecutionTrend_Limit`
**Purpose**: Limit order strategy for strong trends using MACD, Shellington, and MA indicators.

**Key Features**:
- Checks trend alignment across multiple indicators (MACD, Shellington, MA)
- Enters on key support/resistance levels (R1/S1, pivot, or ATR range)
- Take profit: 1H ATR(20)
- Stop loss: 3 × TP
- Risk: 0.3% per trade (configurable)

**Entry Modes**:
1. **Pivot mode**: Entry at pivot points
2. **S1/R1 mode**: Entry at first support/resistance
3. **ATR range mode**: Entry based on ATR calculations

**Symbol-Specific Configurations**:
- **GBPJPY**: Flat trend filter enabled, closes EOD if MA trend reverses
- **USDJPY**: MACD slow filter disabled
- **GBPUSD**: RSI filter enabled, risk reduced on Wednesdays
- **XAUUSD**: Shellington trend enabled, double entry enabled, filters non-farm payroll days
- **EURUSD**: Flat trend enabled
- **BTCUSD/ETHUSD**: Custom ATR adjustments, weekend risk reduction
- **AUDUSD**: Range trade enabled, too-far filter

### 7. `workoutExecutionTrend_MACD_Daily_New`
**Purpose**: MACD-based daily trend continuation strategy.

**Entry Conditions**:
- **Long**:
  - MACD fast > level (symbol-specific)
  - MACD slow > 0 (if enabled)
  - Previous daily close > MA20
  - MACD fast > MACD slow
  - MACD fast increasing
  - Volume conditions met (if enabled)
  - CMF Volume > 0 (if enabled)
  - Weekly baseline alignment (if enabled)
  - Price within 0.2 × daily ATR of previous close

- **Short**:
  - MACD fast < -level
  - MACD slow < 0 (if enabled)
  - Previous daily close < MA20
  - MACD fast < MACD slow
  - MACD fast decreasing
  - Volume conditions met
  - CMF Volume < 0 (if enabled)
  - Weekly baseline alignment (if enabled)
  - Price within 0.2 × daily ATR of previous close

**Filters**:
- MACD BeiLi (divergence) detection
- Volume control (previous volume > prior volume)
- CMF Volume gap
- Weekly baseline confirmation
- ATR volatility filter

**Symbol-Specific MACD Settings**:
- **XTIUSD**: Level 0.35, weekly baseline enabled
- **XAUUSD**: Level 2, max level 10, max level filter enabled
- **GBPJPY**: Level 0.1, ATR and CMF volume enabled
- **GBPUSD**: Level 0.001, CMF volume gap enabled
- **EURUSD**: Level 0, standard MACD (12,26,9)

**Risk Management**:
- Stop loss: Daily Max ATR (symbol-specific multipliers)
- Risk: 1-2% (symbol-specific)
- Split mode: 24 (MACD Daily)
- TP mode: 3 (trailing or fixed)

### 8. `workoutExecutionTrend_MACD_Weekly`
**Purpose**: MACD-based weekly trend strategy.

**Entry Conditions**:
- Uses weekly MACD signals
- Requires weekly trend alignment
- Volume confirmation
- Similar structure to daily MACD but on weekly timeframe

### 9. `workoutExecutionTrend_Ichimoko_Daily_V2`
**Purpose**: Ichimoku-based daily trend strategy.

**Entry Conditions**:
- Uses Ichimoku baseline (Tenkan/Kijun average)
- Weekly baseline confirmation
- Daily close above/below baseline
- Multiple baseline alignment checks

**Exit Conditions**:
- Price breaks opposite baseline
- MACD divergence detected
- 3K trend reversal

### 10. `workoutExecutionTrend_4H_Shellington`
**Purpose**: 4-hour Shellington indicator-based strategy.

**Entry Conditions**:
- Uses 4H Shellington trend signals
- Daily trend alignment required
- Volume confirmation

---

## Order Management Functions

### Order Splitting Strategies

The system uses various order splitting functions to manage position sizing and take profit levels:

#### 1. `splitBuyOrders_ShortTerm` / `splitSellOrders_ShortTerm`
- **Purpose**: Short-term order management with ATR-based calculations
- **Logic**:
  - Calculates lot size based on gap between entry and daily high/low
  - Takes 1/3 of gap as first TP
  - Risk capped to 2% (7× account risk percent)
  - Only trades if gap >= 0.5 × daily ATR
  - Only one order per day

#### 2. `splitBuyOrders_KeyK` / `splitSellOrders_KeyK`
- **Purpose**: KeyK level order splitting
- **Logic**:
  - Three orders: 1×TP, 2×TP, and no TP (let run)
  - Each order: 1/3 of total risk

#### 3. `splitBuyOrders_LongTerm` / `splitSellOrders_LongTerm`
- **Purpose**: Long-term position management
- **Logic**:
  - Two orders: one with TP, one without TP
  - Each order: 1/2 of total risk

#### 4. `splitBuyOrders_4HSwing` / `splitSellOrders_4HSwing`
- **Purpose**: 4-hour swing trading
- **Logic**:
  - Two orders: daily gap and ATR-based TP
  - Each order: 1/2 of total risk
  - Only if price at R1 (long) or S1 (short)

#### 5. `splitBuyOrders_4HSwing_100P` / `splitSellOrders_4HSwing_100P`
- **Purpose**: 100-point TP swing strategy
- **Logic**:
  - Single order with fixed 100-point TP
  - Lot size adjusted based on previous losses (martingale-like recovery)

#### 6. `splitBuyOrders_MACDDaily` / `splitSellOrders_MACDDaily`
- **Purpose**: MACD daily strategy order management
- **Logic**:
  - Multiple orders with different TP levels
  - ATR-based position sizing

#### 7. `splitBuyOrders_Limit` / `splitSellOrders_Limit`
- **Purpose**: Limit order strategy position management
- **Logic**:
  - Multiple TP levels based on ATR
  - Position sizing based on risk percentage

---

## Indicators and Parameters

### Key Indicators Used

1. **MACD (Moving Average Convergence Divergence)**
   - Fast MA: 5-12 periods (symbol-specific)
   - Slow MA: 10-26 periods (symbol-specific)
   - Signal: 5-9 periods (symbol-specific)
   - Used for trend confirmation and entry signals

2. **ATR (Average True Range)**
   - Daily ATR: 20-period
   - Weekly ATR: Calculated from weekly bars
   - Used for stop loss and position sizing

3. **Moving Averages**
   - MA50 Daily: 50-period on daily chart
   - MA20 Daily: 20-period on daily chart
   - MA200/MA50 on 1H and 4H: For trend confirmation

4. **Pivot Points**
   - Daily Pivot, S1, S2, S3, R1, R2, R3
   - Weekly Pivot levels
   - Used for entry and exit levels

5. **BBS (Bollinger Band System)**
   - 15-minute breakout signals
   - Trend execution index
   - Breakout confirmation

6. **Shellington Indicator**
   - 4-hour trend confirmation
   - Used in Limit strategy

7. **Ichimoku**
   - Baseline calculation
   - Trend direction
   - Support/resistance levels

8. **Volume Indicators**
   - CMF (Chaikin Money Flow)
   - Volume comparison (current vs previous)
   - CMF Volume Gap

9. **RSI (Relative Strength Index)**
   - Used for GBPUSD filtering
   - Levels: 20 (oversold), 80 (overbought)

### Key Parameters

- `AUTOBBS_RISK_CAP`: Maximum risk per trade
- `AUTOBBS_KEYK`: KeyK strategy enable/level
- `AUTOBBS_IS_AUTO_MODE`: Auto mode flag
- `AUTOBBS_TP_MODE`: Take profit mode (0=no TP, 1=fixed TP, 2=trailing)
- `AUTOBBS_RANGE`: Range trading enable
- `AUTOBBS_LONG_SHORT_MODE`: Long/short mode selection
- `AUTOBBS_MAX_STRATEGY_RISK`: Maximum strategy risk
- `ACCOUNT_RISK_PERCENT`: Account risk percentage per trade

---

## Risk Management

### Position Sizing

1. **Standard Calculation**:
   ```c
   lots = calculateOrderSize(pParams, direction, entryPrice, takePrice) * risk
   ```

2. **Risk Capping**:
   - Maximum risk per trade: 2-7% (symbol-specific)
   - Uses `calculateOrderSizeWithSpecificRisk()` for caps

3. **Loss Recovery** (4HSwing_100P):
   - Adjusts lot size based on previous losses
   - Formula: `lots = total_pre_lost / takePrice`

### Stop Loss Management

1. **ATR-Based**:
   - Stop loss = Entry ± (Daily ATR × multiplier)
   - Multipliers: 1.0, 1.5, 1.8, 2.0, 3.0 (strategy-specific)

2. **Support/Resistance Based**:
   - Stop loss at daily S/R levels
   - Pivot-based stops

3. **Moving Stops**:
   - `stopMovingBackSL`: Prevents stop loss from moving against profit
   - Trailing stops based on price movement

### Profit Management

1. **Early Closure**:
   - Closes short-term orders if intraday ATR exceeds limits
   - Closes negative positions at end of day (EOD)
   - Closes all positions in range phase

2. **Partial Profit Taking**:
   - Multiple TP levels allow partial closures
   - Some orders have no TP to let winners run

3. **Risk Reduction**:
   - Reduces risk on specific days (e.g., Wednesdays, Fridays)
   - Weekend risk adjustments for crypto pairs

---

## Entry and Exit Logic

### Entry Signal Generation

Entry signals are set via `pIndicators->entrySignal`:
- `1`: Long entry signal
- `-1`: Short entry signal
- `0`: No entry signal

### Entry Conditions Summary

1. **Trend Alignment**: Daily/weekly trend must align with trade direction
2. **Indicator Confirmation**: MACD, MA, or BBS signals must confirm
3. **Price Level**: Must be at appropriate support/resistance
4. **Volume**: Volume confirmation (if enabled)
5. **Volatility**: ATR must be within acceptable range
6. **No Duplicate Orders**: Checks for existing orders at similar price

### Exit Signal Generation

Exit signals are set via `pIndicators->exitSignal`:
- `EXIT_BUY`: Close all buy positions
- `EXIT_SELL`: Close all sell positions
- `EXIT_ALL`: Close all positions
- `EXIT_NONE`: No exit

### Exit Conditions

1. **Trend Reversal**: Daily trend phase changes
2. **MACD Divergence**: MACD crosses opposite direction
3. **Price Break**: Price breaks key support/resistance
4. **Time-Based**: End of day/week closures
5. **Range Phase**: Market enters consolidation
6. **Stop Loss Hit**: Price reaches stop loss level

---

## Symbol-Specific Configurations

### GBPJPY
- Flat trend filter enabled
- Closes EOD if MA trend reverses
- Start hour: 3

### USDJPY
- MACD slow filter disabled
- Closes EOD if MA trend reverses

### GBPUSD
- RSI filter enabled (20/80 levels)
- Risk reduced to 0.6% on Wednesdays
- Start hour: 3

### XAUUSD (Gold)
- Shellington trend enabled
- Double entry enabled
- Filters non-farm payroll days (first Friday)
- Filters Christmas Eve and New Year's Eve
- Risk reduced to 0.5% on Thursdays
- Start hour: 3, Stop hour: 22
- Limit SR1 enabled

### EURUSD
- Flat trend enabled
- MACD slow filter disabled
- Start hour: 3

### BTCUSD / ETHUSD
- Custom ATR adjustments (1% of daily ATR)
- Risk reduced on specific weekdays
- Weekly ATR filter disabled
- Too-far filter enabled (2× limit)
- Double entry enabled
- Too-big spread filter enabled
- MACD periods: 7, 14, 7

### AUDUSD
- Range trade enabled
- Too-far filter enabled
- Stop hour: 17
- MACD slow filter disabled

### XTIUSD (Oil)
- MACD level: 0.35
- Weekly baseline enabled
- Stop loss: 1.8 × daily ATR
- Max risk: 1.5%

---

## Strategy Selection Matrix

| Trend Phase | Primary Strategy | Fallback Strategy | Split Mode |
|------------|------------------|-------------------|------------|
| Beginning UP/DOWN | DailyOpen or Pivot | BBS_BreakOut | 0 or 6 |
| Middle UP/DOWN | DailyOpen or Pivot | BBS_BreakOut | 0 or 6 |
| Middle Retreat | MIDDLE_RETREAT_PHASE | None | 0 |
| Range | Exit All or Adjust SL | None | N/A |

---

## Time-Based Filters

### Trading Hours
- **Start Hour**: Typically 1:00 or 3:00 (symbol-specific)
- **Stop Hour**: Typically 17:00, 22:00, or 23:00 (symbol-specific)
- **Filter Execution TF**: Filters trades during low-liquidity hours

### Day-of-Week Filters
- **Wednesday**: Risk reduction for GBPUSD, XAUUSD
- **Thursday**: Risk reduction for XAUUSD
- **Friday**: Non-farm payroll filter for XAUUSD
- **Weekend**: Risk adjustments for crypto pairs

### Special Date Filters
- **Non-Farm Payroll**: First Friday of month (XAUUSD)
- **Christmas Eve**: December 24 (XAUUSD, BTCUSD, ETHUSD)
- **New Year's Eve**: December 31 (XAUUSD, BTCUSD, ETHUSD)

---

## Best Practices

1. **Monitor Daily Trend Phase**: Strategy selection depends heavily on trend phase
2. **Check Symbol Configuration**: Each symbol has unique parameters
3. **Review Risk Settings**: Ensure risk percentages are appropriate
4. **Watch for Divergence**: MACD BeiLi detection can prevent bad trades
5. **Volume Confirmation**: Enable volume filters for better entry quality
6. **ATR Awareness**: High ATR may trigger early closures
7. **Multiple TP Strategy**: Use split orders to lock in profits at different levels

---

## Troubleshooting

### No Entry Signals
- Check if daily trend phase is appropriate
- Verify indicator alignment (MACD, MA, BBS)
- Ensure price is at support/resistance levels
- Check for duplicate orders at similar price
- Verify trading hours and day filters

### Premature Exits
- Review stop loss settings
- Check for trend phase changes
- Verify MACD divergence detection
- Review time-based exit conditions

### Position Sizing Issues
- Check risk percentage settings
- Verify ATR calculations
- Review risk capping logic
- Check for loss recovery adjustments

---

## Code Structure

### Main File
- **Location**: `dev/TradingStrategies/src/strategies/TrendStrategy.c`
- **Lines**: ~11,196
- **Header**: `dev/TradingStrategies/include/strategies/TrendStrategy.h`

### Key Dependencies
- `Base.h`: Base indicators and trend definitions
- `OrderManagement.h`: Order execution functions
- `EasyTradeCWrapper.hpp`: Easy trade interface
- `ComLib.h`: Common library functions
- `StrategyUserInterface.h`: Strategy interface

---

## Version History

This documentation covers the current implementation as of the codebase review. The strategy has evolved through multiple versions with symbol-specific optimizations and risk management improvements.

---

## Notes

- The code contains Chinese comments (indicated by character encoding issues in some sections)
- Some strategies have multiple versions (e.g., `MACD_Daily`, `MACD_Daily_New`, `MACD_Daily_ver2`)
- Test strategies are included but not documented here
- Regression test strategies exist for validation

---

*Last Updated: Based on codebase review of TrendStrategy.c*

