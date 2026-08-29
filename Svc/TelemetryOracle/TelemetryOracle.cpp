#include "Svc/TelemetryOracle/TelemetryOracle.hpp"
#ifndef FPRIME_STANDALONE
#include <Fw/Types/Assert.hpp>
namespace Svc {
TelemetryOracle::TelemetryOracle(const char* compName): TelemetryOracleComponentBase(compName), m_logic(), m_enabled(Fw::Enabled::ENABLED) {}
TelemetryOracle::~TelemetryOracle() {}
void TelemetryOracle::init(FwSizeType qd, FwEnumStoreType inst){ TelemetryOracleComponentBase::init(qd,inst); }

void TelemetryOracle::schedIn_handler(FwIndexType portNum, U32 context){
  if(!isEnabled()) return;
  // bounded dispatch similar to Health::Run_handler
  for(FwSizeType i=0;i<5;++i){ auto s=this->doDispatch(); if(s==MSG_DISPATCH_EMPTY) break; FW_ASSERT(s==MSG_DISPATCH_OK); }
  for(FwIndexType ch=0; ch < static_cast<FwIndexType>(Svc::TO_MAX_CHANNELS); ++ch){
    ToeResult r = m_logic.runChannel(ch);
    if(!r.computed) continue;
    // telemetry: aggregate + last
    this->tlmWrite_TO_HealthyChannels(m_logic.healthyCount());
    this->tlmWrite_TO_ShiftedChannels(m_logic.shiftedCount());
    U32 insuff = 0; for(U32 c=0;c<Svc::TO_MAX_CHANNELS;++c) if(m_logic.channel(c).state==Svc::ToeState::INSUFFICIENT) ++insuff;
    this->tlmWrite_TO_InsufficientChannels(insuff);
    this->tlmWrite_TO_LastAlpha(r.alpha);
    this->tlmWrite_TO_LastR2(r.r2);
    if(!r.r2_ok){ this->log_ACTIVITY_LO_TO_InsufficientStructure(static_cast<U32>(ch), r.r2); continue; }
    const auto& chan = m_logic.channel(ch);
    if(chan.baseline_set && chan.baseline_samples==0){ // first baseline just set (approx)
      // emit baseline once: check if this was baseline establishment (delta==0 and baseline just set)
      // Logic sets baseline on first R2-ok run; detect via r.delta==0 and previous baseline not set is not tracked here,
      // so we store a per-channel flag via baseline_samples. Simpler: emit when r.delta==0 and not shifted
      if(!r.is_shift) this->log_ACTIVITY_HI_TO_BaselineEstablished(static_cast<U32>(ch), r.alpha, r.r2);
    }
    if(r.is_shift){
      this->log_WARNING_HI_TO_StructuralShift(static_cast<U32>(ch), chan.baseline_alpha, r.alpha, r.delta);
    }
  }
}
void TelemetryOracle::tlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val){
  if(!isEnabled()) return;
  // map id -> channel index via hash; for now low bits
  FwIndexType ch = static_cast<FwIndexType>(id % Svc::TO_MAX_CHANNELS);
  // deserialize F64 from TlmBuffer; fallback to raw bytes if not F64
  F64 sample = 0;
  Fw::SerializeStatus stat = val.deserialize(sample);
  if(stat != Fw::FW_SERIALIZE_OK){ // try U32/I32/F32 overloads
    val.resetDeser(); F32 f32=0; if(val.deserialize(f32)==Fw::FW_SERIALIZE_OK) sample=static_cast<F64>(f32);
  }
  (void)timeTag; (void)portNum;
  m_logic.pushSample(ch, sample);
}
void TelemetryOracle::pingIn_handler(FwIndexType portNum, U32 key){ this->pingOut_out(portNum, key); }

void TelemetryOracle::TO_ENABLE_cmdHandler(FwOpcodeType op, U32 seq, Fw::Enabled en){ m_enabled=en; this->cmdResponse_out(op,seq,Fw::CmdResponse::OK); }
void TelemetryOracle::TO_SET_THRESHOLDS_cmdHandler(FwOpcodeType op, U32 seq, F64 r2, F64 delta){
  if(r2<0||r2>1||delta<0||delta>2){ this->cmdResponse_out(op,seq,Fw::CmdResponse::VALIDATION_ERROR); return; }
  m_logic.setThresholds(r2,delta);
  this->log_ACTIVITY_HI_TO_ThresholdsUpdated(r2,delta);
  this->cmdResponse_out(op,seq,Fw::CmdResponse::OK);
}
void TelemetryOracle::TO_RESET_BASELINE_cmdHandler(FwOpcodeType op, U32 seq, U32 channel){
  if(channel >= Svc::TO_MAX_CHANNELS){ this->cmdResponse_out(op,seq,Fw::CmdResponse::VALIDATION_ERROR); return; }
  m_logic.resetChannel(static_cast<FwIndexType>(channel));
  this->log_ACTIVITY_HI_TO_BaselineReset(channel);
  this->cmdResponse_out(op,seq,Fw::CmdResponse::OK);
}
void TelemetryOracle::TO_RESET_ALL_cmdHandler(FwOpcodeType op, U32 seq){ m_logic.reset(); this->cmdResponse_out(op,seq,Fw::CmdResponse::OK); }
} // ns Svc
#endif
