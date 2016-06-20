#pragma once

#include "fftwf_helpers.h"

#include "stl_wrappers.h"

#include "glog/logging.h"

#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

/// This namespace houses all of the machinery for multiband crossover
/// filtering.
namespace filter {

/// Interface for the most generic boring filter.
class Filter {
public:
    virtual ~Filter() noexcept = default;
    /// Given a vector of data, return a bandpassed version of the data.
    virtual void filter(std::vector<float> &data) = 0;
};

class Lopass : public virtual Filter {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring hipass filter.
class Hipass : public virtual Filter {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring bandpass filter.
class Bandpass : public virtual Filter {
public:
    /// A hipass has mutable lopass, hipass, and samplerate.
    virtual void set_params(float l, float h, float s);
    float lo, hi, sr;
};

class FastConvolution {
public:
    /// An fftconvolover has a constant length.
    /// This means you can reuse it for lots of different convolutions
    /// without reallocating memory, as long as they're all the same size.
    explicit FastConvolution(int FFT_LENGTH);
    virtual ~FastConvolution() noexcept = default;

    template <typename T, typename U>
    std::vector<float> convolve(const T &a, const U &b) {
        CHECK(a.size() + b.size() - 1 == FFT_LENGTH);
        forward_fft(r2c, a, r2c_i, r2c_o, acplx);
        forward_fft(r2c, b, r2c_i, r2c_o, bcplx);

        c2r_i.zero();
        c2r_o.zero();

        auto x = acplx.begin();
        auto y = bcplx.begin();
        auto z = c2r_i.begin();

        for (; z != c2r_i.end(); ++x, ++y, ++z) {
            (*z)[0] += (*x)[0] * (*y)[0] - (*x)[1] * (*y)[1];
            (*z)[1] += (*x)[0] * (*y)[1] + (*x)[1] * (*y)[0];
        }

        fftwf_execute(c2r);

        std::vector<float> ret(c2r_o.begin(), c2r_o.end());

        for (auto &i : ret) {
            i /= FFT_LENGTH;
        }

        return ret;
    }

private:
    template <typename T>
    void forward_fft(const FftwfPlan &plan,
                     const T &data,
                     fftwf_r &i,
                     fftwf_c &o,
                     fftwf_c &results) {
        i.zero();
        proc::copy(data, i.begin());
        fftwf_execute(plan);
        results = o;
    }

    const int FFT_LENGTH;
    const int CPLX_LENGTH;

    fftwf_r r2c_i;
    fftwf_c r2c_o;
    fftwf_c c2r_i;
    fftwf_r c2r_o;
    fftwf_c acplx;
    fftwf_c bcplx;

    FftwfPlan r2c;
    FftwfPlan c2r;
};

class LopassWindowedSinc : public Lopass, public FastConvolution {
public:
    explicit LopassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc hipass filter.
class HipassWindowedSinc : public Hipass, public FastConvolution {
public:
    explicit HipassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc bandpass filter.
class BandpassWindowedSinc : public Bandpass, public FastConvolution {
public:
    explicit BandpassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float l, float h, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// A super-simple biquad filter.
class Biquad : public virtual Filter {
public:
    /// Run the filter foward over some data.
    void filter(std::vector<float> &data) override;

    void set_params(double b0, double b1, double b2, double a1, double a2);

private:
    double b0, b1, b2, a1, a2;
};

template <typename T>
class TwopassFilterWrapper : public T {
public:
    template <typename... Ts>
    TwopassFilterWrapper(Ts &&... ts)
            : T(std::forward<Ts>(ts)...) {
    }

    void filter(std::vector<float> &data) override {
        T::filter(data);
        proc::reverse(data);
        T::filter(data);
        proc::reverse(data);
    }
};

/// Simple biquad bandpass filter.
class BandpassBiquad : public Bandpass, public Biquad {
public:
    void set_params(float l, float h, float s) override;
};

class LinkwitzRileySingleLopass : public Lopass, public Biquad {
public:
    void set_params(float cutoff, float sr) override;
};

class LinkwitzRileySingleHipass : public Hipass, public Biquad {
public:
    void set_params(float cutoff, float sr) override;
};

using LinkwitzRileyLopass = TwopassFilterWrapper<LinkwitzRileySingleLopass>;
using LinkwitzRileyHipass = TwopassFilterWrapper<LinkwitzRileySingleHipass>;

/// A linkwitz-riley filter is just a linear-phase lopass and hipass
/// coupled together.
class LinkwitzRileyBandpass : public Bandpass {
public:
    void set_params(float l, float h, float s) override;
    void filter(std::vector<float> &data) override;

private:
    TwopassFilterWrapper<LinkwitzRileyLopass> lopass;
    TwopassFilterWrapper<LinkwitzRileyHipass> hipass;
};

class DCBlocker : public Biquad {
public:
    DCBlocker();
    constexpr static auto R = 0.995;
};
}  // namespace filter
