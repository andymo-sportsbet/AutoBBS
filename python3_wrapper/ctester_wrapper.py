"""
CTester Wrapper for AsirikuyFrameworkAPI

This module provides a Python 3 wrapper for the AsirikuyFrameworkAPI C library,
specifically designed to integrate with CTester. It provides both:
    - High-level API compatible with CTester's runPortfolioTest interface
    - Low-level API for direct c_runStrategy calls

The wrapper handles:
    - Library loading (platform-specific)
    - Structure conversion (CTester ↔ AsirikuyFrameworkAPI)
    - Per-bar execution loop (AsirikuyFrameworkAPI is per-bar, CTester is batch)
    - Instance management (initInstanceC, deinitInstance)
    - Error handling and propagation
"""

import ctypes
import platform
import os
from ctypes import (
    c_int, c_double, c_char_p, POINTER, byref, CFUNCTYPE, Structure
)

from .structures import (
    CRates, COrderInfo, CRatesInfo, SettingsType, AccountInfoType,
    StrategyResults, AsirikuyReturnCode, MAX_RATES_BUFFERS
)
from .converters import (
    convertRateToCRates, convertOrderInfoToCOrderInfo,
    convertRateInfoToCRatesInfo, prepareRatesArray
)


class AsirikuyFrameworkWrapper:
    """
    Main wrapper class for AsirikuyFrameworkAPI
    
    This class provides access to the AsirikuyFrameworkAPI C library functions
    and manages instance lifecycle.
    """
    
    def __init__(self, library_path=None):
        """
        Initialize the wrapper and load the library.
        
        Args:
            library_path: Optional path to library. If None, uses default paths
                         based on platform.
        """
        self._lib = None
        self._loaded = False
        self._instances = {}  # Track initialized instances
        
        if library_path:
            self.load_library(library_path)
        else:
            self.load_library_default()
    
    def load_library_default(self):
        """Load library using platform-specific default paths."""
        system = platform.system()
        
        if system == "Windows":
            lib_name = "AsirikuyFrameworkAPI.dll"
        elif system == "Linux":
            lib_name = "libAsirikuyFrameworkAPI.so"
        elif system == "Darwin":  # macOS
            lib_name = "libAsirikuyFrameworkAPI.dylib"
        else:
            raise OSError(f"Unsupported platform: {system}")
        
        # Try common library locations
        possible_paths = [
            lib_name,  # Current directory
            f"./bin/{lib_name}",
            f"./lib/{lib_name}",
            f"./build/gmake/x64/Release/{lib_name}",
            f"./build/gmake/x64/Debug/{lib_name}",
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                self.load_library(path)
                return
        
        raise FileNotFoundError(
            f"Could not find {lib_name}. Tried: {', '.join(possible_paths)}"
        )
    
    def load_library(self, library_path):
        """
        Load the AsirikuyFrameworkAPI shared library.
        
        Args:
            library_path: Path to the library file
            
        Raises:
            OSError: If library cannot be loaded
        """
        if not os.path.exists(library_path):
            raise FileNotFoundError(f"Library not found: {library_path}")
        
        try:
            self._lib = ctypes.CDLL(library_path)
            self._loaded = True
            self._setup_function_signatures()
        except OSError as e:
            raise OSError(f"Failed to load library {library_path}: {e}")
    
    def _setup_function_signatures(self):
        """Set up function signatures for ctypes."""
        if not self._loaded:
            return
        
        # initInstanceC
        # int __stdcall initInstanceC(int instanceId, int isTesting, char* pAsirikuyConfig, char* pAccountName)
        self._lib.initInstanceC.argtypes = [c_int, c_int, c_char_p, c_char_p]
        self._lib.initInstanceC.restype = c_int
        
        # deinitInstance
        # void __stdcall deinitInstance(int instanceId)
        self._lib.deinitInstance.argtypes = [c_int]
        self._lib.deinitInstance.restype = None
        
        # c_runStrategy
        # int __stdcall c_runStrategy(double* pInSettings, char* pInTradeSymbol, ...)
        self._lib.c_runStrategy.argtypes = [
            POINTER(c_double),      # pInSettings
            c_char_p,               # pInTradeSymbol
            c_char_p,               # pInAccountCurrency
            c_char_p,               # pInBrokerName
            c_char_p,               # pInRefBrokerName
            POINTER(c_int),         # pInCurrentBrokerTime
            POINTER(c_int),         # pInOpenOrdersCount
            POINTER(COrderInfo),    # pInOrderInfo
            POINTER(c_double),      # pInAccountInfo
            POINTER(c_double),      # pInBidAsk
            POINTER(CRatesInfo),     # pInRatesInfo
            POINTER(CRates),        # pInRates_0
            POINTER(CRates),        # pInRates_1
            POINTER(CRates),        # pInRates_2
            POINTER(CRates),        # pInRates_3
            POINTER(CRates),        # pInRates_4
            POINTER(CRates),        # pInRates_5
            POINTER(CRates),        # pInRates_6
            POINTER(CRates),        # pInRates_7
            POINTER(CRates),        # pInRates_8
            POINTER(CRates),        # pInRates_9
            POINTER(c_double),      # pOutResults
        ]
        self._lib.c_runStrategy.restype = c_int
        
        # getFrameworkVersion
        # void __stdcall getFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix)
        self._lib.getFrameworkVersion.argtypes = [POINTER(c_int), POINTER(c_int), POINTER(c_int)]
        self._lib.getFrameworkVersion.restype = None
    
    def initInstanceC(self, instance_id, is_testing, config_path, account_name=""):
        """
        Initialize a strategy instance.
        
        Args:
            instance_id: Instance ID
            is_testing: 1 for backtesting, 0 for live trading
            config_path: Path to Asirikuy config file
            account_name: Account name (optional)
            
        Returns:
            int: AsirikuyReturnCode (0 = SUCCESS)
            
        Raises:
            RuntimeError: If library not loaded
        """
        if not self._loaded:
            raise RuntimeError("Library not loaded")
        
        # Convert strings to bytes for Python 3
        config_path_bytes = config_path.encode('utf-8') if isinstance(config_path, str) else config_path
        account_name_bytes = account_name.encode('utf-8') if isinstance(account_name, str) else account_name
        
        result = self._lib.initInstanceC(
            c_int(instance_id),
            c_int(is_testing),
            config_path_bytes,
            account_name_bytes
        )
        
        if result == AsirikuyReturnCode.SUCCESS:
            self._instances[instance_id] = {
                'is_testing': is_testing,
                'config_path': config_path,
                'account_name': account_name
            }
        
        return result
    
    def deinitInstance(self, instance_id):
        """
        Deinitialize a strategy instance.
        
        Args:
            instance_id: Instance ID
        """
        if not self._loaded:
            return
        
        if instance_id in self._instances:
            self._lib.deinitInstance(c_int(instance_id))
            del self._instances[instance_id]
    
    def c_runStrategy(self, settings, symbol, account_currency, broker_name, ref_broker_name,
                     current_broker_time, open_orders_count, order_info, account_info,
                     bid_ask, rates_info, rates_array, results):
        """
        Execute strategy for one bar/candle.
        
        Args:
            settings: Settings array (64 doubles)
            symbol: Trade symbol (string)
            account_currency: Account currency (string)
            broker_name: Broker name (string)
            ref_broker_name: Reference broker name (string)
            current_broker_time: Current broker time (int pointer)
            open_orders_count: Number of open orders (int pointer)
            order_info: Array of COrderInfo structures
            account_info: Account info array (10 doubles)
            bid_ask: Bid/ask array
            rates_info: Array of CRatesInfo structures (up to 10)
            rates_array: Array of CRates arrays (up to 10 rate buffers)
            results: Output results array (will be populated)
            
        Returns:
            int: AsirikuyReturnCode (0 = SUCCESS)
            
        Raises:
            RuntimeError: If library not loaded
        """
        if not self._loaded:
            raise RuntimeError("Library not loaded")
        
        # Convert strings to bytes for Python 3
        symbol_bytes = symbol.encode('utf-8') if isinstance(symbol, str) else symbol
        account_currency_bytes = account_currency.encode('utf-8') if isinstance(account_currency, str) else account_currency
        broker_name_bytes = broker_name.encode('utf-8') if isinstance(broker_name, str) else broker_name
        ref_broker_name_bytes = ref_broker_name.encode('utf-8') if isinstance(ref_broker_name, str) else ref_broker_name
        
        # Prepare rates pointers (up to 10 buffers)
        rates_pointers = [None] * MAX_RATES_BUFFERS
        for i in range(min(len(rates_array), MAX_RATES_BUFFERS)):
            if rates_array[i] is not None:
                rates_pointers[i] = byref(rates_array[i])
            else:
                rates_pointers[i] = None
        
        result = self._lib.c_runStrategy(
            settings,
            symbol_bytes,
            account_currency_bytes,
            broker_name_bytes,
            ref_broker_name_bytes,
            current_broker_time,
            open_orders_count,
            order_info,
            account_info,
            bid_ask,
            rates_info,
            rates_pointers[0] if rates_pointers[0] else None,
            rates_pointers[1] if rates_pointers[1] else None,
            rates_pointers[2] if rates_pointers[2] else None,
            rates_pointers[3] if rates_pointers[3] else None,
            rates_pointers[4] if rates_pointers[4] else None,
            rates_pointers[5] if rates_pointers[5] else None,
            rates_pointers[6] if rates_pointers[6] else None,
            rates_pointers[7] if rates_pointers[7] else None,
            rates_pointers[8] if rates_pointers[8] else None,
            rates_pointers[9] if rates_pointers[9] else None,
            results
        )
        
        return result
    
    def getFrameworkVersion(self):
        """
        Get framework version.
        
        Returns:
            tuple: (major, minor, bugfix) version numbers
        """
        if not self._loaded:
            raise RuntimeError("Library not loaded")
        
        major = c_int()
        minor = c_int()
        bugfix = c_int()
        
        self._lib.getFrameworkVersion(byref(major), byref(minor), byref(bugfix))
        
        return (major.value, minor.value, bugfix.value)
    
    def is_loaded(self):
        """Check if library is loaded."""
        return self._loaded
    
    def get_instances(self):
        """Get list of initialized instance IDs."""
        return list(self._instances.keys())


# Global wrapper instance (lazy initialization)
_global_wrapper = None


def get_wrapper(library_path=None):
    """
    Get or create global wrapper instance.
    
    Args:
        library_path: Optional library path (only used on first call)
        
    Returns:
        AsirikuyFrameworkWrapper: Global wrapper instance
    """
    global _global_wrapper
    if _global_wrapper is None:
        _global_wrapper = AsirikuyFrameworkWrapper(library_path)
    return _global_wrapper


# Convenience functions matching CTester interface
def initInstanceC(instance_id, is_testing, config_path, account_name=""):
    """Convenience function for initInstanceC."""
    wrapper = get_wrapper()
    return wrapper.initInstanceC(instance_id, is_testing, config_path, account_name)


def deinitInstance(instance_id):
    """Convenience function for deinitInstance."""
    wrapper = get_wrapper()
    return wrapper.deinitInstance(instance_id)


def c_runStrategy(settings, symbol, account_currency, broker_name, ref_broker_name,
                 current_broker_time, open_orders_count, order_info, account_info,
                 bid_ask, rates_info, rates_array, results):
    """Convenience function for c_runStrategy."""
    wrapper = get_wrapper()
    return wrapper.c_runStrategy(
        settings, symbol, account_currency, broker_name, ref_broker_name,
        current_broker_time, open_orders_count, order_info, account_info,
        bid_ask, rates_info, rates_array, results
    )


# High-level API compatible with CTester's runPortfolioTest
# This will be implemented in execution_engine.py
def runPortfolioTest(*args, **kwargs):
    """
    High-level API compatible with CTester's runPortfolioTest.
    
    This function provides a batch execution interface that internally
    uses per-bar execution (c_runStrategy) to maintain compatibility
    with CTester's existing code.
    
    TODO: Implement in execution_engine.py
    """
    raise NotImplementedError("runPortfolioTest will be implemented in execution_engine.py")

