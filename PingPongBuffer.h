#ifndef PINGPONGBUFFER_H
#define PINGPONGBUFFER_H

#include <vector>
#include <iostream>

class PingPongBuffer
{
public:
    typedef void (*FullBufferCallback)(std::vector<short>);

    PingPongBuffer(int capacity);

    void add_data(const std::vector<short> &data);

    std::vector<short> get_current_buffer() const;

    bool is_full() const;

    void set_on_buffer_a_full_callback(FullBufferCallback callback);

    void set_on_buffer_b_full_callback(FullBufferCallback callback);

private:
    int capacity_;
    std::vector<short> buffer_a_;
    std::vector<short> buffer_b_;
    std::vector<short> *current_buffer_;
    int write_offset_;
    int read_offset_;
    bool buffer_a_full_;
    bool buffer_b_full_;
    FullBufferCallback on_buffer_a_full_callback_;
    FullBufferCallback on_buffer_b_full_callback_;

    void switch_buffer();
};

#endif // PINGPONGBUFFER_H
