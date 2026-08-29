#ifndef Svc_TelemetryOracle_TelemetryOracleLogic_hpp
#define Svc_TelemetryOracle_TelemetryOracleLogic_hpp
#include "DfaProcessor.hpp"

namespace Svc {

static constexpr FwSizeType TO_MAX_CHANNELS = 8;
static constexpr FwSizeType TO_WINDOW = 1024; // 1024 => std 0.041, 2048 => 0.032. 1024 saves 32KB vs 2048.
static constexpr F64 TO_R2_THRESH = 0.85;
static constexpr F64 TO_DELTA_THRESH = 0.15;

enum class ToeState { UNINIT=0, COLLECTING=1, BASELINE_SET=2, HEALTHY=3, SHIFTED=4, INSUFFICIENT=5 };

struct ToeChannel {
    F64 window[TO_WINDOW] = {};
    FwSizeType count = 0;      // samples collected (wraps)
    FwSizeType head = 0;       // write head for circular buffer
    bool filled = false;
    DfaResult last = {};
    F64 baseline_alpha = 0.5;
    F64 baseline_r2 = 0.0;
    bool baseline_set = false;
    ToeState state = ToeState::UNINIT;
    U32 baseline_samples = 0;
    // adaptive: EWMA of alpha and variance for per-channel auto delta
    F64 ewma_alpha = 0.5;
    F64 ewma_var = 0.0;
    U32 ewma_count = 0;
    bool adaptive = true;
};

struct ToeResult {
    bool computed = false;
    bool r2_ok = false;
    bool is_shift = false;
    F64 alpha = 0.5;
    F64 r2 = 0.0;
    F64 delta = 0.0;
    U32 healthy = 0;
    U32 shifted = 0;
    U32 insufficient = 0;
};

class TelemetryOracleLogic {
public:
    TelemetryOracleLogic(F64 r2_thresh = TO_R2_THRESH, F64 delta_thresh = TO_DELTA_THRESH);
    // bounded, no heap
    void reset();
    void resetChannel(FwIndexType ch);
    // push sample for channel 0..TO_MAX_CHANNELS-1; returns false if ch out of range
    bool pushSample(FwIndexType ch, F64 value);
    // run DFA for channel; call from Run_handler. Returns per-channel result.
    ToeResult runChannel(FwIndexType ch);
    // run all channels, aggregate counts
    ToeResult runAll();
    // accessors for test
    const ToeChannel& channel(FwIndexType ch) const { return m_channels[ch]; }
    FwSizeType windowSize() const { return TO_WINDOW; }
    U32 healthyCount() const { return m_healthy; }
    U32 shiftedCount() const { return m_shifted; }
    void setThresholds(F64 r2, F64 delta) { m_r2_thresh=r2; m_delta_thresh=delta; }
private:
    ToeChannel m_channels[TO_MAX_CHANNELS] = {};
    DfaProcessor m_dfa{TO_WINDOW};
    F64 m_r2_thresh;
    F64 m_delta_thresh;
    U32 m_healthy = 0;
    U32 m_shifted = 0;
    void recomputeAggregates();
};

} // ns Svc
#endif
