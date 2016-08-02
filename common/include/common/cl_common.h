#pragma once

#include "cl_traits.h"

class compute_context final {
public:
    compute_context();

    cl::Context get_context() const;
    cl::Device get_device() const;

private:
    cl::Context context;
    cl::Device device;
};

template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool read_only) {
    return cl::Buffer(context, std::begin(t), std::end(t), read_only);
}

template <typename T>
T read_single_value(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t index) {
    T ret;
    queue.enqueueReadBuffer(
            buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &ret);
    return ret;
}

template <typename T>
void write_single_value(cl::CommandQueue& queue,
                        cl::Buffer& buffer,
                        size_t index,
                        T val) {
    queue.enqueueWriteBuffer(
            buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &val);
}

