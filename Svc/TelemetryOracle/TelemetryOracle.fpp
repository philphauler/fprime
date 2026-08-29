module Svc {
  @ Structural health monitor via Detrended Fluctuation Analysis (DFA-1, Peng 1994).
  @ Window TO_WINDOW=1024 (std 0.041) or 2048 (std 0.032). N=256 unusable (std 0.15).
  @ Pow2 boxes {16,32,64,128,256,512} verified: white mean 0.504, brown 1.41.
  queued component TelemetryOracle {

    # ----------------------------------------------------------------------
    # Ports
    # ----------------------------------------------------------------------
    sync input port schedIn: Svc.Sched
    guarded input port tlmRecv: Fw.Tlm
    async input port pingIn: Svc.Ping
    output port pingOut: Svc.Ping

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------
    command recv port CmdDisp
    command reg port CmdReg
    command resp port CmdStatus
    event port Log
    text event port LogText
    time get port Time
    telemetry port Tlm

    # ----------------------------------------------------------------------
    # Commands
    # ----------------------------------------------------------------------
    async command TO_ENABLE(enabled: Fw.Enabled) opcode 0x0
    async command TO_SET_THRESHOLDS(r2_thresh: F64, delta_thresh: F64) opcode 0x1
    async command TO_RESET_BASELINE(channel: U32) opcode 0x2
    async command TO_RESET_ALL() opcode 0x3

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------
    event TO_BaselineEstablished(channel: U32, alpha: F64, r2: F64) severity activity high id 0x0 format "CH {} baseline alpha={:.3f} R2={:.4f}"
    event TO_InsufficientStructure(channel: U32, r2: F64) severity activity low id 0x1 format "CH {} R2={:.4f} below thresh"
    event TO_StructuralShift(channel: U32, baseline: F64, current: F64, delta: F64) severity warning high id 0x2 format "CH {} shift baseline={:.3f} cur={:.3f} delta={:+.3f}" throttle 5
    event TO_ThresholdsUpdated(r2: F64, delta: F64) severity activity high id 0x3 format "thresholds R2={:.3f} delta={:.3f}"
    event TO_BaselineReset(channel: U32) severity activity high id 0x4 format "CH {} baseline reset"

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------
    telemetry TO_HealthyChannels: U32 id 0x0
    telemetry TO_ShiftedChannels: U32 id 0x1
    telemetry TO_InsufficientChannels: U32 id 0x2
    telemetry TO_LastAlpha: F64 id 0x3
    telemetry TO_LastR2: F64 id 0x4
  }
}
