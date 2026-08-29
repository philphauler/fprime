####
# Svc/TelemetryOracle CMakeLists
####
register_fprime_module(
  AUTOCODER_INPUTS
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracle.fpp"
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracle.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracleLogic.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DfaProcessor.hpp"
  HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracle.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracleLogic.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/DfaProcessor.hpp"
)

### UTs ###
register_fprime_ut(
  AUTOCODER_INPUTS
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracle.fpp"
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/TelemetryOracleTester.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/TelemetryOracleTester.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/TelemetryOracleTestMain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/TelemetryOracleLogic.cpp"
)
