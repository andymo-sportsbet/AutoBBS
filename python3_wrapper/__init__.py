"""
Python 3 Wrapper for AsirikuyFrameworkAPI

This package provides a Python 3 interface to the AsirikuyFrameworkAPI C library,
specifically designed for CTester integration.

Modules:
    - ctester_wrapper: Main wrapper providing CTester-compatible interface
    - structures: ctypes structure definitions matching CTesterDefines.h
    - converters: Functions to convert between CTester and AsirikuyFrameworkAPI structures
    - execution_engine: Per-bar execution loop for strategy testing
"""

__version__ = "0.1.0"
__author__ = "AutoBBS Project"

# Import main wrapper for convenience
try:
    from .ctester_wrapper import (
        AsirikuyFrameworkWrapper,
        runPortfolioTest,
        initInstanceC,
        deinitInstance,
        c_runStrategy,
        get_wrapper
    )
    
    __all__ = [
        'AsirikuyFrameworkWrapper',
        'runPortfolioTest',
        'initInstanceC',
        'deinitInstance',
        'c_runStrategy',
        'get_wrapper',
    ]
except ImportError:
    # Allow import even if dependencies aren't available
    __all__ = []

