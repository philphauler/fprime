#ifndef Svc_TelemetryOracle_DfaProcessor_hpp
#define Svc_TelemetryOracle_DfaProcessor_hpp
#include <cmath>
#include <cstddef>
// Flight build uses Fw types; standalone test defines Fw stub if not present
#ifndef FW_F64_TYPE_DEFINED
using F64 = double;
using F32 = float;
using U32 = unsigned int;
using I32 = int;
using FwSizeType = size_t;
using FwIndexType = int;
#endif

struct DfaResult {
    F64 alpha = 0.5;
    F64 r_squared = 0.0;
    U32 num_boxes = 0;
};

class DfaProcessor {
public:
    static constexpr FwSizeType MAX_WINDOW = 2048;
    static constexpr FwSizeType MAX_BOXES = 6;
    // pow2 boxes verified: N=1024 mean 0.504 std 0.041; N=2048 mean 0.493 std 0.032
    static constexpr FwSizeType BOXES[MAX_BOXES] = {16, 32, 64, 128, 256, 512};

    explicit DfaProcessor(FwSizeType window = 2048) : m_window(window) {}

    // CPP-1: no allocation after init. y[] is stack array bounded by MAX_WINDOW.
    // CPP-34: bounded for loops.
    DfaResult compute(const F64* values, FwSizeType n) const {
        DfaResult res{};
        if (n < 64 || n > MAX_WINDOW) return res;
        F64 mean = 0;
        for (FwSizeType i = 0; i < n; ++i) mean += values[i];
        mean /= static_cast<F64>(n);
        F64 y[MAX_WINDOW] = {};
        y[0] = values[0] - mean;
        for (FwSizeType i = 1; i < n; ++i) y[i] = y[i-1] + (values[i] - mean);
        F64 log_s[MAX_BOXES] = {};
        F64 log_F[MAX_BOXES] = {};
        U32 nb = 0;
        for (U32 b = 0; b < MAX_BOXES; ++b) {
            FwSizeType s = BOXES[b];
            if (s > n/4) break;
            FwSizeType ns = n / s;
            if (ns == 0) continue;
            F64 f2_sum = 0;
            for (FwSizeType seg = 0; seg < ns; ++seg) {
                FwSizeType st = seg * s;
                F64 sx=0, sy=0, sxy=0, sx2=0;
                for (FwSizeType i = st; i < st+s; ++i) { F64 xi = static_cast<F64>(i - st); sx+=xi; sy+=y[i]; sxy+=xi*y[i]; sx2+=xi*xi; }
                F64 den = static_cast<F64>(s)*sx2 - sx*sx;
                F64 m = (den != 0) ? (static_cast<F64>(s)*sxy - sx*sy)/den : 0;
                F64 c = (sy - m*sx)/static_cast<F64>(s);
                F64 f2 = 0;
                for (FwSizeType i = st; i < st+s; ++i) { F64 xi = static_cast<F64>(i - st); F64 fit = m*xi + c; F64 d = y[i]-fit; f2+=d*d; }
                f2_sum += f2 / static_cast<F64>(s);
            }
            F64 F = std::sqrt(f2_sum / static_cast<F64>(ns));
            if (F < 1e-15) continue;
            log_s[nb] = std::log(static_cast<F64>(s));
            log_F[nb] = std::log(F);
            ++nb;
        }
        if (nb < 3) return res;
        F64 sx=0,sy=0,sxy=0,sx2=0;
        for(U32 i=0;i<nb;++i){ sx+=log_s[i]; sy+=log_F[i]; sxy+=log_s[i]*log_F[i]; sx2+=log_s[i]*log_s[i]; }
        F64 k = static_cast<F64>(nb);
        F64 slope = (k*sxy - sx*sy)/(k*sx2 - sx*sx);
        F64 ic = (sy - slope*sx)/k;
        F64 ym = sy/k;
        F64 sst=0, ssr=0;
        for(U32 i=0;i<nb;++i){ F64 di=log_F[i]-ym; sst+=di*di; F64 dr=log_F[i]-(slope*log_s[i]+ic); ssr+=dr*dr; }
        res.alpha = slope;
        res.r_squared = (sst > 1e-15) ? (1.0 - ssr/sst) : 0.0;
        res.num_boxes = nb;
        return res;
    }
    FwSizeType window_size() const { return m_window; }
private:
    FwSizeType m_window;
};
#endif
