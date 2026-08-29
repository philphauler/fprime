# Svc::TelemetryOracle SDD

## Purpose
Passive structural-health monitor that detects behavioral degradation before
threshold monitors fire. A degrading bearing changes vibration *pattern* before
amplitude exceeds a limit. DFA exponent alpha captures that pattern.

## Theory
DFA-1 (Peng 1994): integrate, partition into boxes, local linear detrend per box,
fluctuation F(s), slope of log F vs log s = alpha.
- alpha~0.5 white, 0.5-1.0 long-range correlated (healthy complex), ~1.0 1/f, >1.0 non-stationary.

## Design
- `TO_WINDOW=1024` (std 0.041 white). 2048 gives std 0.032 but doubles RAM (64KB -> 128KB for 8 ch). 256 unusable (std 0.15).
- Pow2 boxes `{16,32,64,128,256,512}` (tighter variance than log-spaced).
- Per-channel circular buffer, bounded, no heap after init (CPP-1/CPP-25).
- `schedIn` runs DFA per filled channel; `tlmRecv` pushes samples.
- R2 gate (default 0.85) -> INSUFFICIENT; delta gate (default 0.15) -> SHIFTED else HEALTHY.
- Commands: TO_ENABLE, TO_SET_THRESHOLDS, TO_RESET_BASELINE, TO_RESET_ALL.

## Verification
Standalone clang test: `fprime_clone/Svc/TelemetryOracle/test/ut/` mean white 0.493-0.504, brown 1.41, shift injection detected.
