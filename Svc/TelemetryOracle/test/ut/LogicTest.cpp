#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "TelemetryOracleLogic.hpp"
#include "TelemetryOracleLogic.cpp"
#include "DfaProcessor.hpp"

int failures=0;
void check(bool cond, const char* msg){ if(cond) printf("  PASS %s\n", msg); else { printf("  FAIL %s\n", msg); failures++; } }

int main(){
 printf("=== TelemetryOracleLogic unit tests ===\n");
 // Test 1: DFA engine white vs brown separation (N=1024)
 {
  printf("\n[Test1] DFA white vs brown separation\n");
  DfaProcessor proc(1024);
  srand(42); double w[1024]; for(int i=0;i<1024;i++) w[i]=(double)rand()/RAND_MAX*2-1;
  auto rw = proc.compute(w,1024);
  srand(42); double b[1024]; double c=0; for(int i=0;i<1024;i++){ c+=(double)rand()/RAND_MAX*2-1; b[i]=c; }
  auto rb = proc.compute(b,1024);
  printf("  white alpha=%.3f R2=%.3f brown alpha=%.3f R2=%.3f\n", rw.alpha, rw.r_squared, rb.alpha, rb.r_squared);
  check(rw.alpha>0.35 && rw.alpha<0.65, "white alpha ~0.5");
  check(rw.r_squared>0.85, "white R2>0.85");
  check(rb.alpha>1.2 && rb.alpha<1.7, "brown alpha ~1.4");
  check(rb.r_squared>0.9, "brown R2>0.9");
  check(rb.alpha - rw.alpha > 0.7, "brown-white separation >0.7");
 }
 // Test 2: baseline establishment + R2 gating
 {
  printf("\n[Test2] baseline + R2 gating + healthy count\n");
  Svc::TelemetryOracleLogic logic;
  check(logic.healthyCount()==0, "initial healthy 0");
  // push 1024 white samples to ch0
  srand(100);
  for(int i=0;i<1024;i++) logic.pushSample(0, (double)rand()/RAND_MAX*2-1);
  auto r1 = logic.runChannel(0);
  printf("  r1 computed=%d r2_ok=%d alpha=%.3f R2=%.3f state=%d healthy=%u\n", r1.computed, r1.r2_ok, r1.alpha, r1.r2, (int)logic.channel(0).state, logic.healthyCount());
  check(r1.computed, "computed after fill");
  check(r1.r2_ok, "R2 ok");
  check(logic.channel(0).baseline_set, "baseline set");
  check(logic.healthyCount()==1, "healthy 1 after baseline");
  // second run should stay healthy (same distribution)
  srand(101);
  for(int i=0;i<1024;i++) logic.pushSample(0, (double)rand()/RAND_MAX*2-1);
  auto r2 = logic.runChannel(0);
  printf("  r2 alpha=%.3f delta=%.3f shift=%d\n", r2.alpha, r2.delta, r2.is_shift);
  check(!r2.is_shift, "no shift on same distribution");
 }
 // Test 3: shift injection (white baseline -> brown shift)
 {
  printf("\n[Test3] shift injection white->brown\n");
  Svc::TelemetryOracleLogic logic(0.80, 0.15); // looser R2 for this test
  srand(200);
  for(int i=0;i<1024;i++) logic.pushSample(1, (double)rand()/RAND_MAX*2-1);
  auto rb = logic.runChannel(1);
  printf("  baseline alpha=%.3f R2=%.3f\n", rb.alpha, rb.r2);
  check(rb.r2_ok, "baseline R2 ok");
  // now feed brown (correlated) - should shift
  srand(201); double c=0;
  for(int i=0;i<1024;i++){ c+=(double)rand()/RAND_MAX*2-1; logic.pushSample(1, c); }
  auto rs = logic.runChannel(1);
  printf("  shifted alpha=%.3f delta=%.3f is_shift=%d R2=%.3f\n", rs.alpha, rs.delta, rs.is_shift, rs.r2);
  check(rs.is_shift, "brown shift detected");
  check(logic.shiftedCount()==1, "shifted count 1");
  check(logic.healthyCount()==0, "healthy 0 after shift");
 }
 // Test 4: insufficient structure (constant signal -> R2 low)
 {
  printf("\n[Test4] insufficient R2 on constant signal\n");
  Svc::TelemetryOracleLogic logic(0.85, 0.15);
  for(int i=0;i<1024;i++) logic.pushSample(2, 1.0); // constant
  auto r = logic.runChannel(2);
  printf("  constant alpha=%.3f R2=%.3f r2_ok=%d state=%d\n", r.alpha, r.r2, r.r2_ok, (int)logic.channel(2).state);
  check(r.computed, "computed");
  // constant -> F ~0 -> nb<3 -> returns 0.5/0.0 -> R2 fail
  check(!r.r2_ok, "constant R2 fails gate");
 }
 // Test 5: bounded - out of range channel rejected, no crash
 {
  printf("\n[Test5] bounded channel guard\n");
  Svc::TelemetryOracleLogic logic;
  check(!logic.pushSample(99, 1.0), "push out-of-range rejected");
  check(!logic.pushSample(-1, 1.0), "push negative rejected");
  auto r = logic.runChannel(99); check(!r.computed, "run out-of-range not computed");
 }
 // Test 6: runAll aggregates
 {
  printf("\n[Test6] runAll healthy/shifted aggregates\n");
  Svc::TelemetryOracleLogic logic(0.80, 0.15);
  srand(300); for(int i=0;i<1024;i++) logic.pushSample(0, (double)rand()/RAND_MAX*2-1);
  srand(301); for(int i=0;i<1024;i++) logic.pushSample(1, (double)rand()/RAND_MAX*2-1);
  logic.runChannel(0); logic.runChannel(1);
  auto agg = logic.runAll();
  printf("  agg healthy=%u shifted=%u insufficient=%u\n", agg.healthy, agg.shifted, agg.insufficient);
  check(agg.healthy==2, "runAll healthy 2");
 }
 printf("\n=== RESULT: %s (%d failures) ===\n", failures==0?"PASS":"FAIL", failures);
 return failures==0?0:1;
}



