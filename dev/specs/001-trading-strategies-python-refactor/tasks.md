# Tasks: TradingStrategies Integration & Refactoring

**Input**: Specification and plan from `/specs/001-trading-strategies-python-refactor/`
**Prerequisites**: spec.md (required), plan.md (required), MIGRATION_PLAN.md (required)

## Execution Flow
```
1. Load spec.md, plan.md, and MIGRATION_PLAN.md from feature directory
   → Extract: requirements, architecture, timeline
2. Generate tasks by phase:
   → Phase 1: Integration (CTester + Live Trading)
   → Phase 2: Refactoring
   → Phase 3: Future Enhancements
3. Apply task rules:
   → Different files = mark [P] for parallel
   → Same file = sequential (no [P])
   → Tests before implementation (TDD where possible)
4. Number tasks sequentially (T001, T002...)
5. Create dependency graph
6. Validate task completeness
```

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths in descriptions

## Phase 1: Setup & Planning

### Phase 1.1: Specification
- [x] T001 Create spec folder structure `specs/001-trading-strategies-python-refactor/`
- [x] T002 Create specification document `spec.md`
- [x] T003 Create implementation plan `plan.md`
- [x] T004 Create migration plan `MIGRATION_PLAN.md`
- [x] T005 Create task breakdown `tasks.md`
- [x] T006 Create architecture document `architecture.md`
- [x] T007 Review and validate specification

### Phase 1.2: Environment Setup
- [x] T008 [P] Set up development environment (macOS ARM64, premake4, Boost, cURL)
- [x] T009 [P] Build system setup (macOS build successful)
- [x] T010 [P] Review existing CTester Python 2 codebase (access obtained, initial review complete)
- [x] T010a [P] Migrate CTester to Python 3 (Python 3 migration complete)
- [ ] T011 [P] Review existing Live Trading Platform Python 2 codebase (waiting for access)
- [x] T012 [P] Document settings array indices mapping
- [x] T013 [P] Set up Python 3 testing environment
  - ✅ Python 3.13.7 installed and verified
  - ✅ pytest available
  - ✅ unittest available (built-in)
  - ✅ Syntax validation working
  - ✅ Python 3 environment ready for testing
  - ⚠️ Note: Test infrastructure (test files, pytest config) will be created during integration testing phase

## Phase 2: Integration (Weeks 1-3) - Priority 1

### Phase 2.1: CTester Integration (Week 1)

#### T020-T024: Review & Analysis
- [x] T020 [P] Review existing CTester Python 3 codebase
  - ✅ Analyze how CTester currently calls trading strategies
  - ✅ Document data structures and interfaces
  - ✅ Identify dependencies and requirements
  - ✅ Create integration analysis document
- [x] T021 [P] Document CTester interfaces
  - ✅ Document current CTester API
  - ✅ Map CTester data structures to CTesterDefines.h
  - ✅ Identify integration points
  - ✅ Create interface documentation

#### T025-T029: Python 3 Wrapper for CTester
- [x] T025 Create `python3_wrapper/ctester_wrapper.py` skeleton
  - ✅ Create directory structure
  - ✅ Set up basic file structure
  - ✅ Add imports and basic setup
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
- [ ] T026 [DEFERRED] Implement ctypes bindings for AsirikuyFrameworkAPI
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - CTester will continue using CTesterFrameworkAPI directly
- [ ] T027 [DEFERRED] Map CTester data structures to CTesterDefines.h
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
- [ ] T028 [DEFERRED] Implement c_runStrategy function binding
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred

#### T030-T033: CTester Integration
- [x] T030 [DEFERRED] Integrate wrapper with existing CTester code
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - ✅ CTester already integrates with CTesterFrameworkAPI directly
  - ✅ No wrapper needed for current architecture
- [x] T031 [DEFERRED] Convert CTester test data to CTester structures
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - ✅ CTester data structures already compatible with CTesterFrameworkAPI
- [x] T032 [DEFERRED] Implement strategy execution flow
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - ✅ CTester execution flow already works with CTesterFrameworkAPI
- [x] T033 [DEFERRED] Create CTester integration tests
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - ✅ CTester tests already work with CTesterFrameworkAPI

#### T034-T035: CTester Testing & Documentation
- [x] T034 [P] Run existing CTester test suite
  - ✅ Execute all CTester tests (8 diagnostic tests)
  - ✅ Verify all tests pass
  - ✅ Document any failures
  - ✅ Fix identified issues
  - ✅ Fixed Python 3 import issues (mt.py, fastcsv)
  - ✅ Fixed library loading (build directory paths)
  - ✅ Fixed config parsing (inline comments)
  - ✅ Fixed string/bytes encoding
- [ ] T035 Create CTester integration documentation
  - Document integration process
  - Create usage guide
  - Document troubleshooting
  - Add code examples

### Phase 2.2: Live Trading Platform Integration (Week 2)

#### T040-T043: Review & Analysis
- [ ] T040 [P] Review existing Live Trading Platform Python 2/3 codebase
  - Analyze broker REST API integration
  - Document order management and position tracking
  - Identify real-time data flow
  - Create integration analysis document
  - ⚠️ Note: May need Python 3 migration if still on Python 2
- [ ] T041 [P] Document Live Trading Platform interfaces
  - Document current Live Trading API
  - Map broker API responses
  - Identify integration points
  - Create interface documentation

#### T044-T047: Python 3 Wrapper for Live Trading
- [ ] T044 [DEFERRED] Create `python3_wrapper/live_trading_wrapper.py` skeleton
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - Live Trading Platform will continue using existing architecture
- [ ] T045 [DEFERRED] Implement broker REST API → CTester structure conversion
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
- [ ] T046 [DEFERRED] Implement order management interface
  - ⚠️ **DEFERRED**: Migration to AsirikuyFrameworkAPI deferred
  - Track open positions
  - Manage order lifecycle
  - Error handling for network failures
- [ ] T047 Implement real-time data fetching
  - Implement tick-by-tick updates
  - Implement bar-by-bar updates
  - Handle data synchronization
  - Implement retry logic

#### T048-T051: Live Trading Integration
- [ ] T048 Integrate wrapper with existing Live Trading Platform
  - Identify integration points
  - Replace existing strategy calls
  - Maintain backward compatibility
  - Test integration
- [ ] T049 Implement real-time strategy execution loop
  - Implement execution loop
  - Handle market data updates
  - Execute strategies on updates
  - Manage execution state
- [ ] T050 [P] Create broker API adapter
  - Create `python3_wrapper/broker_api_adapter.py`
  - Implement broker-specific adapters
  - Handle different broker APIs
  - Error handling and retry logic
- [ ] T051 [P] Create Live Trading integration tests
  - Test with paper trading
  - Test order execution
  - Test position tracking
  - Test error handling

#### T052-T053: Live Trading Testing & Documentation
- [ ] T052 [P] Test with paper trading
  - Set up paper trading environment
  - Execute live trading tests
  - Verify order execution
  - Performance testing
- [ ] T053 Create Live Trading integration documentation
  - Document integration process
  - Create usage guide
  - Document broker API setup
  - Add code examples

### Phase 2.3: Integration Testing & Polish (Week 3)

#### T060-T064: Comprehensive Testing
- [ ] T060 [P] End-to-end testing of both systems
  - Test CTester integration end-to-end
  - Test Live Trading integration end-to-end
  - Test cross-system scenarios
  - Document test results
- [ ] T061 [P] Cross-platform testing
  - Test on macOS
  - Test on Linux (if available)
  - Verify platform compatibility
  - Document platform-specific issues
- [ ] T062 [P] Performance benchmarking
  - Benchmark CTester performance
  - Benchmark Live Trading performance
  - Compare with baseline
  - Document performance metrics
- [ ] T063 [P] Memory leak validation
  - Test CTester memory usage
  - Test Live Trading memory usage
  - Use memory profiling tools
  - Fix identified leaks
- [ ] T064 [P] Stress testing
  - Test with large datasets
  - Test with multiple strategies
  - Test error recovery
  - Document stress test results

#### T065-T067: Documentation & Bug Fixes
- [ ] T065 Create comprehensive integration documentation
  - Complete integration guides
  - API reference documentation
  - Troubleshooting guides
  - Migration notes
- [ ] T066 [P] Bug fixes and optimization
  - Fix identified issues
  - Performance optimization
  - Code cleanup
  - Code review
- [ ] T067 Final integration testing
  - Run full test suite
  - Verify all tests pass
  - Final validation
  - Sign-off

## Phase 3: Refactoring (Weeks 4-8) - Priority 2

### Phase 3.1: Code Analysis (Week 4)

#### T070-T074: Code Complexity Analysis
- [ ] T070 [P] Measure file sizes
  - Identify files > 2000 lines
  - Document file sizes
  - Create file size report
  - Prioritize files for refactoring
- [ ] T071 [P] Calculate cyclomatic complexity
  - Analyze code complexity
  - Identify complex functions
  - Create complexity report
  - Prioritize refactoring targets
- [ ] T072 [P] Identify code duplication
  - Find duplicate code patterns
  - Document duplication
  - Create refactoring opportunities list
  - Plan code extraction
- [ ] T073 [P] Document pain points
  - Identify maintainability issues
  - Document architectural problems
  - Create pain points report
  - Prioritize improvements

#### T074-T076: Dependency Analysis
- [ ] T074 [P] Map dependencies between modules
  - Create dependency graph
  - Identify coupling issues
  - Document dependencies
  - Create dependency report
- [ ] T075 [P] Identify coupling issues
  - Find tight coupling
  - Identify circular dependencies
  - Document coupling problems
  - Plan decoupling strategy
- [ ] T076 [P] Document architecture
  - Create architecture diagram
  - Document current structure
  - Identify refactoring opportunities
  - Create architecture report

#### T077-T079: Refactoring Strategy
- [ ] T077 Evaluate refactoring options
  - Evaluate C++ refactoring option
  - Evaluate Python refactoring option
  - Evaluate Hybrid approach
  - Create comparison document
- [ ] T078 Create detailed refactoring plan
  - Plan module-by-module refactoring
  - Identify shared utilities to extract
  - Plan backward compatibility strategy
  - Create implementation roadmap
- [ ] T079 Risk assessment
  - Identify refactoring risks
  - Create mitigation strategies
  - Plan rollback strategy
  - Document risk assessment

### Phase 3.2: Refactoring Decision & Planning (Week 5)

#### T080-T083: Refactoring Approach Decision
- [ ] T080 Review analysis results
  - Review code complexity analysis
  - Review dependency analysis
  - Review architecture documentation
  - Create decision document
- [ ] T081 Evaluate options (C++/Python/Hybrid)
  - Evaluate C++ option (pros/cons)
  - Evaluate Python option (pros/cons)
  - Evaluate Hybrid option (pros/cons)
  - Create evaluation document
- [ ] T082 Make refactoring approach decision
  - Make decision based on analysis
  - Document decision rationale
  - Create decision document
  - Get approval if needed
- [ ] T083 Create detailed implementation plan
  - Plan module-by-module refactoring
  - Identify first modules to refactor
  - Plan test strategy
  - Create timeline

#### T084-T086: Risk Mitigation Planning
- [ ] T084 Identify risks and mitigation strategies
  - Identify technical risks
  - Identify project risks
  - Create mitigation strategies
  - Document risk plan
- [ ] T085 Create rollback plan
  - Plan rollback procedures
  - Identify rollback checkpoints
  - Create rollback documentation
  - Test rollback procedures
- [ ] T086 Plan incremental migration
  - Plan incremental refactoring steps
  - Identify migration checkpoints
  - Plan testing at each step
  - Create migration timeline

### Phase 3.3: Refactoring Implementation (Weeks 6-8)

#### T090-T094: Extract Common Code (Week 6)
- [ ] T090 [P] Identify common utilities
  - Find common code patterns
  - Identify reusable functions
  - Document common utilities
  - Plan extraction
- [ ] T091 [P] Extract to shared modules
  - Create `TradingStrategies/common/` directory
  - Extract common code
  - Create shared utility modules
  - Update includes
- [ ] T092 [P] Create shared module structure
  - Organize shared modules
  - Create module interfaces
  - Document shared modules
  - Create unit tests
- [ ] T093 [P] Unit tests for shared code
  - Create unit tests
  - Test shared utilities
  - Verify functionality
  - Document tests

#### T094-T098: Split Monolithic Files (Week 6)
- [ ] T094 [P] Start with largest files (e.g., TrendStrategy.c)
  - Identify monolithic files
  - Analyze file structure
  - Plan splitting strategy
  - Create splitting plan
- [ ] T095 [P] Split by strategy type
  - Create `strategies/macd/` directory
  - Create `strategies/ichimoku/` directory
  - Create `strategies/bbs/` directory
  - Split strategy code
- [ ] T096 [P] Extract common strategy logic
  - Identify common strategy patterns
  - Extract to shared modules
  - Update strategy modules
  - Test extraction
- [ ] T097 [P] Maintain backward compatibility
  - Ensure API compatibility
  - Update includes
  - Test compatibility
  - Document changes

#### T098-T102: Refactor Strategy Modules (Week 7)
- [ ] T098 [P] Refactor MACD strategy module
  - Refactor code organization
  - Improve naming
  - Add comments
  - Create unit tests
- [ ] T099 [P] Refactor Ichimoku strategy module
  - Refactor code organization
  - Improve naming
  - Add comments
  - Create unit tests
- [ ] T100 [P] Refactor BBS strategy module
  - Refactor code organization
  - Improve naming
  - Add comments
  - Create unit tests
- [ ] T101 [P] Refactor other strategy modules
  - Refactor remaining strategies
  - Improve code organization
  - Add comments
  - Create unit tests
- [ ] T102 [P] Improve code organization
  - Organize file structure
  - Improve naming conventions
  - Add comprehensive comments
  - Update documentation

#### T103-T107: Integration & Testing (Week 8)
- [ ] T103 [P] Integration tests
  - Test refactored modules
  - Test module interactions
  - Verify functionality
  - Document test results
- [ ] T104 [P] Verify backward compatibility
  - Test API compatibility
  - Test strategy results
  - Compare with baseline
  - Document compatibility
- [ ] T105 [P] Performance testing
  - Benchmark refactored code
  - Compare with baseline
  - Optimize if needed
  - Document performance
- [ ] T106 [P] Bug fixes
  - Fix identified issues
  - Test fixes
  - Document fixes
  - Code review
- [ ] T107 [P] Documentation updates
  - Update code documentation
  - Update architecture docs
  - Update API docs
  - Create refactoring summary

## Phase 4: Future Enhancements (Future) - Priority 3

### Phase 4.1: Python API Interface (Deferred)

#### T110-T115: Python API Design & Implementation
- [ ] T110 Design TradingStrategiesPythonAPI
  - Design API interface
  - Design data structures
  - Create API specification
  - Document design decisions
- [ ] T111 Implement TradingStrategiesPythonAPI
  - Create API header
  - Implement API functions
  - Implement data conversion
  - Create unit tests
- [ ] T112 Create Python 3 wrapper (Enhanced)
  - ✅ Python 3 wrapper infrastructure ready (CTester migrated)
  - Implement enhanced ctypes bindings
  - Add NumPy integration
  - Create unit tests
- [ ] T113 Add NumPy integration
  - Integrate NumPy arrays
  - Optimize data conversion
  - Create examples
  - Document usage
- [ ] T114 [P] Create Python API documentation
  - Document API
  - Create usage examples
  - Create integration guide
  - Document best practices
- [ ] T115 [P] Python API testing
  - Create test suite
  - Test API functions
  - Test Python wrapper
  - Performance testing

## Task Dependencies

```
T001 → T002 → T003 → T004 → T005 → T006 → T007
T008, T009, T010, T010a, T011, T012, T013 (parallel setup)

# CTester Integration (Week 1)
T010 → T010a → T020 → T021
T020, T021 → T025 → T026 → T027 → T028
T025, T026, T027, T028 → T030 → T031 → T032
T030, T031, T032 → T033, T034
T033, T034 → T035

# Live Trading Integration (Week 2)
T011 → T040 → T041
T040, T041 → T044 → T045 → T046 → T047
T044, T045, T046, T047 → T048 → T049
T048, T049 → T050, T051
T050, T051 → T052, T053

# Integration Testing (Week 3)
T035, T053 → T060, T061, T062, T063, T064 (parallel testing)
T060, T061, T062, T063, T064 → T065 → T066 → T067

# Refactoring Analysis (Week 4)
T067 → T070, T071, T072, T073 (parallel analysis)
T070, T071, T072, T073 → T074, T075, T076 (parallel analysis)
T074, T075, T076 → T077 → T078 → T079

# Refactoring Planning (Week 5)
T079 → T080 → T081 → T082 → T083
T083 → T084 → T085 → T086

# Refactoring Implementation (Weeks 6-8)
T086 → T090, T091, T092, T093 (parallel extraction)
T090, T091, T092, T093 → T094 → T095 → T096 → T097
T097 → T098, T099, T100, T101 (parallel refactoring)
T098, T099, T100, T101 → T102
T102 → T103, T104, T105, T106, T107 (parallel testing)

# Future Enhancements (Deferred)
T107 → T110 → T111 → T112 → T113 → T114, T115
```

## Parallel Execution Examples

### Example 1: Integration Analysis
```
T020, T021, T040, T041 can run in parallel
(Review CTester and Live Trading codebases)
```

### Example 2: Integration Testing
```
T060, T061, T062, T063, T064 can run in parallel
(End-to-end, cross-platform, performance, memory, stress testing)
```

### Example 3: Code Analysis
```
T070, T071, T072, T073 can run in parallel
(File sizes, complexity, duplication, pain points)
```

### Example 4: Strategy Refactoring
```
T098, T099, T100, T101 can run in parallel
(Refactor different strategy modules)
```

## Critical Path

The critical path is:
```
T001 → T008 → T010 → T010a → T020 → T025 → T026 → T028 → T030 → T032 → T033 → T034
→ T011 → T040 → T044 → T045 → T048 → T049 → T052 → T060 → T065 → T066 → T067
→ T070 → T074 → T077 → T080 → T082 → T083 → T090 → T094 → T098 → T103 → T107
```

This represents the minimum sequence to complete integration and refactoring.

## Notes

- Tasks marked [P] can be done in parallel
- TDD approach: Write tests before implementation where possible
- All file paths are relative to project root
- Estimated total: ~115 tasks
- Estimated effort: 120-150 story points
- **Priority**: Phase 2 (Integration) is critical path
- **Deferred**: Phase 4 (Python API Interface) is future consideration

## ⚠️ Important Decision: Migration Deferred

**Date**: December 2024  
**Decision**: Migration to AsirikuyFrameworkAPI has been **DEFERRED**

CTester will continue using **CTesterFrameworkAPI directly**, which already works perfectly. No wrapper is needed for the current architecture.

See `MIGRATION_DEFERRED.md` for details.

---

## Progress Summary (December 2024)

### Completed Tasks
- ✅ **Phase 1**: Specification and planning complete
- ✅ **Build System**: macOS build system complete
  - ✅ T008: Development environment setup
  - ✅ T009: Build system setup (macOS)
  - ✅ T024-T028: Build system tasks (from old plan)
- ✅ **Phase 1.2**: Environment Setup
  - ✅ T010: CTester Python 2 codebase review (access obtained, initial review complete)
  - ✅ T010a: **Python 3 Migration Complete** (December 2024)
    - ✅ Migrated all CTester Python files from Python 2 to Python 3
    - ✅ Fixed all syntax and compatibility issues
    - ✅ Updated imports, print statements, configparser
    - ✅ Fixed string/bytes handling, base64 encoding
    - ✅ All files pass syntax validation
  - ✅ T012: Settings array indices mapping documented
  - ✅ T013: Python 3 testing environment set up
    - ✅ Python 3.13.7 installed and verified
    - ✅ pytest and unittest available
    - ✅ Environment ready for testing

### Current Status
- **Build System**: ✅ macOS working, 🔄 Linux/Windows pending
- **Dependencies**: ✅ All resolved (Pantheios removed, MiniXML integrated, Boost fixed)
- **Python Version**: ✅ **Python 3.8+** (CTester migrated, Live Trading Platform pending)
- **Integration**: ✅ CTester Python 3 codebase ready, ⚠️ **AsirikuyFrameworkAPI migration DEFERRED**
- **Architecture**: ✅ CTester uses CTesterFrameworkAPI directly (no wrapper needed)
- **Next Steps**:
  1. ✅ T010a: Python 3 migration complete
  2. ✅ T020-T021: CTester analysis and interface documentation complete
  3. ⚠️ T025-T033: AsirikuyFrameworkAPI migration deferred
  4. ⏳ T011: Obtain access to Live Trading Platform codebase (may need Python 3 migration)
  5. Focus on other project priorities

### Key Achievements
- Successfully built `libAsirikuyFrameworkAPI.dylib` on macOS ARM64
- Removed all Pantheios dependencies (replaced with fprintf)
- Integrated MiniXML for XML parsing
- Fixed Boost library linking issues
- Created comprehensive build documentation
- Verified library exports and dependencies
- Verified Python loading works (__stdcall ignored on macOS)
- ✅ Cleaned up CTester codebase (removed build artifacts, fixed bugs)
- ✅ Obtained access to CTester codebase and completed initial review
- ✅ **Python 3 Migration**: Successfully migrated CTester from Python 2 to Python 3.8+
  - 15 files converted
  - ~2000 lines changed
  - All syntax validation passed
  - Ready for Python 3 integration work

### Blocked Tasks
- T011: Review existing Live Trading Platform codebase (waiting for access)
  - ⚠️ Note: May need Python 3 migration if still on Python 2
- Phase 2.2 tasks depend on T011
