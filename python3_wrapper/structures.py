"""
ctypes Structure Definitions for AsirikuyFrameworkAPI

This module defines ctypes structures that match the C structures defined in
CTesterDefines.h. These structures are used for interfacing with the
AsirikuyFrameworkAPI C library.

References:
    - dev/AsirikuyFrameworkAPI/include/CTester/CTesterDefines.h
    - dev/AsirikuyFrameworkAPI/include/CTester/CTesterTradingStrategiesAPI.h
"""

from ctypes import Structure, c_double, c_int, c_char_p, POINTER

# Error codes from AsirikuyDefines.h
class AsirikuyReturnCode:
    """Return codes from AsirikuyFrameworkAPI"""
    SUCCESS = 0
    INVALID_CURRENCY = 3000
    UNKNOWN_SYMBOL = 3001
    SYMBOL_TOO_SHORT = 3002
    PARSE_SYMBOL_FAILED = 3003
    NO_CONVERSION_SYMBOLS = 3004
    NULL_POINTER = 3005
    INVALID_STRATEGY = 3006
    NOT_ENOUGH_MARGIN = 3007
    SPREAD_TOO_WIDE = 3008
    NOT_ENOUGH_RATES_DATA = 3009
    WORST_CASE_SCENARIO = 3010
    NORMALIZE_BARS_FAILED = 3011
    INIT_LOG_FAILED = 3012
    DEINIT_LOG_FAILED = 3013
    ZERO_DIVIDE = 3014
    TA_LIB_ERROR = 3015
    INVALID_TIME_OFFSET = 3016
    INVALID_PARAMETER = 3017
    NN_TRAINING_FAILED = 3018
    UNKNOWN_TIMEZONE = 3019
    LOCAL_TZ_MISMATCH = 3020
    BROKER_TZ_MISMATCH = 3021
    TOO_MANY_INSTANCES = 3022
    INVALID_CONFIG = 3023
    MISSING_CONFIG = 3024
    INIT_XML_FAILED = 3025
    UNKNOWN_INSTANCE_ID = 3026
    INSUFFICIENT_MEMORY = 3027
    WAIT_FOR_INIT = 3028


class CRates(Structure):
    """
    Rate structure matching CTesterDefines.h CRates_t
    
    Represents a single OHLCV candle. Note: This structure does NOT include
    swap data (unlike CTester's Rate structure which includes shortSwap/longSwap).
    
    Fields:
        open (double): Open price
        high (double): High price
        low (double): Low price
        close (double): Close price
        volume (double): Volume
        time (int): Timestamp
    """
    _fields_ = [
        ("open", c_double),
        ("high", c_double),
        ("low", c_double),
        ("close", c_double),
        ("volume", c_double),
        ("time", c_int),
    ]


class COrderInfo(Structure):
    """
    Order information structure matching CTesterDefines.h COrderInfo_t
    
    Represents a trading order/trade. Note: Field name is "expiriation" (typo in C code).
    
    Fields:
        ticket (double): Order ticket number
        instanceId (double): Strategy instance ID
        type (double): Order type (0=BUY, 1=SELL)
        openTime (double): Order open time (timestamp)
        closeTime (double): Order close time (timestamp)
        stopLoss (double): Stop loss price
        takeProfit (double): Take profit price
        expiriation (double): Expiration time (note: typo in C code)
        openPrice (double): Order open price
        closePrice (double): Order close price
        lots (double): Order lot size
        profit (double): Order profit
        commission (double): Commission
        swap (double): Swap
        isOpen (double): Is order open (1.0=open, 0.0=closed)
    """
    _fields_ = [
        ("ticket", c_double),
        ("instanceId", c_double),
        ("type", c_double),
        ("openTime", c_double),
        ("closeTime", c_double),
        ("stopLoss", c_double),
        ("takeProfit", c_double),
        ("expiriation", c_double),  # Note: typo in C code
        ("openPrice", c_double),
        ("closePrice", c_double),
        ("lots", c_double),
        ("profit", c_double),
        ("commission", c_double),
        ("swap", c_double),
        ("isOpen", c_double),
    ]


class CRatesInfo(Structure):
    """
    Rate information structure matching CTesterDefines.h CRatesInfo_t
    
    Contains metadata about rate arrays (timeframe, size, precision).
    Note: Field names use "timeframe" (lowercase) vs CTester's "TimeFrame" (camelCase).
    
    Fields:
        isEnabled (double): Is this rate buffer enabled (1.0=enabled, 0.0=disabled)
        requiredTimeframe (double): Required timeframe
        totalBarsRequired (double): Total bars required
        actualTimeframe (double): Actual timeframe
        ratesArraySize (double): Size of rates array
        point (double): Point value (price precision)
        digits (double): Number of decimal digits
    """
    _fields_ = [
        ("isEnabled", c_double),
        ("requiredTimeframe", c_double),  # Note: lowercase vs CTester's "requiredTimeFrame"
        ("totalBarsRequired", c_double),
        ("actualTimeframe", c_double),   # Note: lowercase vs CTester's "actualTimeFrame"
        ("ratesArraySize", c_double),
        ("point", c_double),
        ("digits", c_double),
    ]


# Settings array type (64 doubles)
SettingsType = c_double * 64

# AccountInfo array type (10 doubles)
AccountInfoType = c_double * 10

# StrategyResults output structure (matches StrategyResults from AsirikuyDefines.h)
# This is returned in pOutResults parameter of c_runStrategy
class StrategyResults(Structure):
    """
    Strategy results structure returned by c_runStrategy
    
    This structure is populated by the C library in the pOutResults parameter.
    """
    _fields_ = [
        ("ticketNumber", c_double),
        ("tradingSignals", c_double),
        ("lots", c_double),
        ("entryPrice", c_double),
        ("brokerSL", c_double),
        ("brokerTP", c_double),
        ("internalSL", c_double),
        ("internalTP", c_double),
        ("useTrailingSL", c_double),
        ("expirationTime", c_double),
    ]


# Maximum number of rate buffers
MAX_RATES_BUFFERS = 10

