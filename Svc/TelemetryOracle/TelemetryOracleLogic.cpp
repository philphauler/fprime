#include "TelemetryOracleLogic.hpp"
#include <cstring>
namespace Svc {
TelemetryOracleLogic::TelemetryOracleLogic(F64 r2, F64 delta): m_r2_thresh(r2), m_delta_thresh(delta) { reset(); }
void TelemetryOracleLogic::reset(){ for(U32 i=0;i<TO_MAX_CHANNELS;++i) resetChannel(static_cast<FwIndexType>(i)); m_healthy=0; m_shifted=0; }
void TelemetryOracleLogic::resetChannel(FwIndexType ch){
 if(ch<0 || ch>=static_cast<FwIndexType>(TO_MAX_CHANNELS)) return;
 ToeChannel &c = m_channels[ch];
 for(FwSizeType i=0;i<TO_WINDOW;++i) c.window[i]=0;
 c.count=0; c.head=0; c.filled=false; c.last={}; c.baseline_alpha=0.5; c.baseline_r2=0; c.baseline_set=false; c.state=ToeState::COLLECTING; c.baseline_samples=0;
 c.ewma_alpha=0.5; c.ewma_var=0; c.ewma_count=0;
}
bool TelemetryOracleLogic::pushSample(FwIndexType ch, F64 v){
 if(ch<0 || ch>=static_cast<FwIndexType>(TO_MAX_CHANNELS)) return false;
 ToeChannel &c = m_channels[ch];
 c.window[c.head]=v;
 c.head = (c.head+1)%TO_WINDOW;
 if(!c.filled && c.count+1 >= TO_WINDOW) c.filled=true;
 if(c.count < TO_WINDOW) ++c.count;
 if(c.state==ToeState::UNINIT) c.state=ToeState::COLLECTING;
 return true;
}
ToeResult TelemetryOracleLogic::runChannel(FwIndexType ch){
 ToeResult r{};
 if(ch<0 || ch>=static_cast<FwIndexType>(TO_MAX_CHANNELS)) return r;
 ToeChannel &c = m_channels[ch];
 if(!c.filled) return r; // not enough samples
 // linearize circular buffer into contiguous array for DFA (stack, bounded)
 F64 linear[TO_WINDOW]={};
 for(FwSizeType i=0;i<TO_WINDOW;++i) linear[i]=c.window[(c.head+i)%TO_WINDOW];
 DfaResult d = m_dfa.compute(linear, TO_WINDOW);
 c.last=d; r.computed=true; r.alpha=d.alpha; r.r2=d.r_squared;
 r.r2_ok = (d.r_squared >= m_r2_thresh);
 if(!r.r2_ok){ c.state=ToeState::INSUFFICIENT; r.insufficient=1; recomputeAggregates(); return r; }
 if(!c.baseline_set){
   c.baseline_alpha=d.alpha; c.baseline_r2=d.r_squared; c.baseline_set=true;
   c.ewma_alpha=d.alpha; c.ewma_var=0; c.ewma_count=1;
   c.state=ToeState::HEALTHY; recomputeAggregates(); return r;
 }
 // adaptive: EWMA var, threshold = max(fixed, 3*sigma)
 F64 thresh = m_delta_thresh;
 if(c.adaptive && c.ewma_count>=3){
   F64 sigma = c.ewma_var>0 ? std::sqrt(c.ewma_var) : 0;
   F64 adapt = 3.0*sigma;
   if(adapt > thresh) thresh = adapt;
   if(thresh < 0.08) thresh = 0.08; // floor
   if(thresh > 0.40) thresh = 0.40; // ceiling
 }
 r.delta = d.alpha - c.baseline_alpha;
 if(r.delta<0) r.delta=-r.delta;
 r.is_shift = (r.delta > thresh);
 if(!r.is_shift){
   // update EWMA only on healthy samples (not shifts)
   F64 diff = d.alpha - c.ewma_alpha;
   c.ewma_alpha = 0.90*c.ewma_alpha + 0.10*d.alpha;
   c.ewma_var = 0.90*c.ewma_var + 0.10*diff*diff;
   if(c.ewma_count<100) ++c.ewma_count;
 }
 c.state = r.is_shift ? ToeState::SHIFTED : ToeState::HEALTHY;
 recomputeAggregates(); r.healthy=m_healthy; r.shifted=m_shifted;
 return r;
}
ToeResult TelemetryOracleLogic::runAll(){
 ToeResult agg{}; agg.computed=true;
 for(FwIndexType ch=0; ch<static_cast<FwIndexType>(TO_MAX_CHANNELS); ++ch){ ToeResult cr=runChannel(ch); if(cr.r2_ok && !cr.is_shift) agg.healthy++; if(cr.is_shift) agg.shifted++; if(cr.computed && !cr.r2_ok) agg.insufficient++; }
 agg.healthy=m_healthy; agg.shifted=m_shifted;
 return agg;
}
void TelemetryOracleLogic::recomputeAggregates(){
 U32 h=0,s=0; for(U32 i=0;i<TO_MAX_CHANNELS;++i){ if(m_channels[i].state==ToeState::HEALTHY) ++h; if(m_channels[i].state==ToeState::SHIFTED) ++s; } m_healthy=h; m_shifted=s;
}
}
