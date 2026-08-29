#ifndef Svc_TelemetryOracle_hpp
#define Svc_TelemetryOracle_hpp
#include "Svc/TelemetryOracle/TelemetryOracleLogic.hpp"

// When autocoded, include base; standalone test defines FPRIME_STANDALONE to skip.
#ifndef FPRIME_STANDALONE
#include "Svc/TelemetryOracle/TelemetryOracleComponentAc.hpp"
namespace Svc {
class TelemetryOracle final : public TelemetryOracleComponentBase {
  friend class TelemetryOracleTester;
public:
  explicit TelemetryOracle(const char* compName);
  ~TelemetryOracle();
  void init(FwSizeType queueDepth, FwEnumStoreType instance);
private:
  // handlers
  void schedIn_handler(FwIndexType portNum, U32 context) override;
  void tlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) override;
  void pingIn_handler(FwIndexType portNum, U32 key) override;
  // commands
  void TO_ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::Enabled enabled) override;
  void TO_SET_THRESHOLDS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, F64 r2_thresh, F64 delta_thresh) override;
  void TO_RESET_BASELINE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 channel) override;
  void TO_RESET_ALL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

  TelemetryOracleLogic m_logic;
  Fw::Enabled m_enabled = Fw::Enabled::ENABLED;
  bool isEnabled() const { return m_enabled == Fw::Enabled::ENABLED; }
};
} // ns Svc
#else
// standalone alias so #include "TelemetryOracle.hpp" still works in LogicTest
namespace Svc { using TelemetryOracle = TelemetryOracleLogic; }
#endif
#endif
