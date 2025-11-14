"""
Conversion Functions for CTester ↔ AsirikuyFrameworkAPI Structures

This module provides functions to convert between CTester's Python structures
and AsirikuyFrameworkAPI's C structures (CTesterDefines.h).

Key Differences:
    - CTester Rate includes swap data (shortSwap, longSwap), CRates does not
    - Field name case differences: requiredTimeFrame vs requiredTimeframe
    - Field name typo: expiration vs expiriation
"""

from ctypes import Structure, POINTER, byref
from .structures import CRates, COrderInfo, CRatesInfo


def convertRateToCRates(rate, bar_index=None):
    """
    Convert CTester Rate structure to CRates structure.
    
    Args:
        rate: CTester Rate structure or array element
        bar_index: Optional index if rate is an array element
    
    Returns:
        CRates: Converted structure
        
    Note:
        Swap data (shortSwap, longSwap) is not included in CRates.
        Swap must be handled separately in order processing.
    """
    crates = CRates()
    
    if bar_index is not None:
        # If rate is an array, access the specific bar
        crates.open = rate[bar_index].open
        crates.high = rate[bar_index].high
        crates.low = rate[bar_index].low
        crates.close = rate[bar_index].close
        crates.volume = rate[bar_index].volume
        crates.time = rate[bar_index].time
    else:
        # If rate is a single structure
        crates.open = rate.open
        crates.high = rate.high
        crates.low = rate.low
        crates.close = rate.close
        crates.volume = rate.volume
        crates.time = rate.time
    
    return crates


def convertOrderInfoToCOrderInfo(order_info):
    """
    Convert CTester OrderInfo structure to COrderInfo structure.
    
    Args:
        order_info: CTester OrderInfo structure
    
    Returns:
        COrderInfo: Converted structure
        
    Note:
        Field name "expiration" maps to "expiriation" (typo in C code).
    """
    c_order = COrderInfo()
    c_order.ticket = order_info.ticket
    c_order.instanceId = order_info.instanceId
    c_order.type = order_info.type
    c_order.openTime = order_info.openTime
    c_order.closeTime = order_info.closeTime
    c_order.stopLoss = order_info.stopLoss
    c_order.takeProfit = order_info.takeProfit
    c_order.expiriation = order_info.expiration  # Note: typo in C structure
    c_order.openPrice = order_info.openPrice
    c_order.closePrice = order_info.closePrice
    c_order.lots = order_info.lots
    c_order.profit = order_info.profit
    c_order.commission = order_info.commission
    c_order.swap = order_info.swap
    c_order.isOpen = order_info.isOpen
    
    return c_order


def convertCOrderInfoToOrderInfo(c_order):
    """
    Convert COrderInfo structure to CTester OrderInfo structure.
    
    Args:
        c_order: COrderInfo structure from C library
    
    Returns:
        OrderInfo: CTester OrderInfo structure (from ctester.include.asirikuy)
    """
    # Import here to avoid circular dependency
    from ctester.include.asirikuy import OrderInfo
    
    order_info = OrderInfo()
    order_info.ticket = c_order.ticket
    order_info.instanceId = c_order.instanceId
    order_info.type = c_order.type
    order_info.openTime = c_order.openTime
    order_info.closeTime = c_order.closeTime
    order_info.stopLoss = c_order.stopLoss
    order_info.takeProfit = c_order.takeProfit
    order_info.expiration = c_order.expiriation  # Note: typo in C structure
    order_info.openPrice = c_order.openPrice
    order_info.closePrice = c_order.closePrice
    order_info.lots = c_order.lots
    order_info.profit = c_order.profit
    order_info.commission = c_order.commission
    order_info.swap = c_order.swap
    order_info.isOpen = c_order.isOpen
    
    return order_info


def convertRateInfoToCRatesInfo(rate_info):
    """
    Convert CTester RateInfo structure to CRatesInfo structure.
    
    Args:
        rate_info: CTester RateInfo structure
    
    Returns:
        CRatesInfo: Converted structure
        
    Note:
        Field name case differences: requiredTimeFrame → requiredTimeframe
    """
    c_rate_info = CRatesInfo()
    c_rate_info.isEnabled = rate_info.isEnabled
    c_rate_info.requiredTimeframe = rate_info.requiredTimeFrame  # Case difference
    c_rate_info.totalBarsRequired = rate_info.totalBarsRequired
    c_rate_info.actualTimeframe = rate_info.actualTimeFrame  # Case difference
    c_rate_info.ratesArraySize = rate_info.ratesArraySize
    c_rate_info.point = rate_info.point
    c_rate_info.digits = rate_info.digits
    
    return c_rate_info


def convertCRatesInfoToRateInfo(c_rate_info):
    """
    Convert CRatesInfo structure to CTester RateInfo structure.
    
    Args:
        c_rate_info: CRatesInfo structure from C library
    
    Returns:
        RateInfo: CTester RateInfo structure (from ctester.include.asirikuy)
    """
    # Import here to avoid circular dependency
    from ctester.include.asirikuy import RateInfo
    
    rate_info = RateInfo()
    rate_info.isEnabled = c_rate_info.isEnabled
    rate_info.requiredTimeFrame = c_rate_info.requiredTimeframe  # Case difference
    rate_info.totalBarsRequired = c_rate_info.totalBarsRequired
    rate_info.actualTimeFrame = c_rate_info.actualTimeframe  # Case difference
    rate_info.ratesArraySize = c_rate_info.ratesArraySize
    rate_info.point = c_rate_info.point
    rate_info.digits = c_rate_info.digits
    
    return rate_info


def prepareRatesArray(rate_array, num_bars):
    """
    Prepare array of CRates structures from CTester Rate array.
    
    Args:
        rate_array: CTester Rate array
        num_bars: Number of bars in array
    
    Returns:
        List of CRates structures
    """
    crates_array = []
    for i in range(num_bars):
        crates = convertRateToCRates(rate_array, i)
        crates_array.append(crates)
    
    return crates_array

