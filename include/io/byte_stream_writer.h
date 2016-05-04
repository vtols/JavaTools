#ifndef BYTE_STREAM_WRITER_H
#define BYTE_STREAM_WRITER_H

#include <cstddef>
#include <cstdint>

class ByteStreamWriter
{
public:
    virtual ~ByteStreamWriter() = 0;

    virtual void write(uint8_t *buffer, size_t count) = 0;
    void write(uint8_t buffer);
    void write(uint16_t buffer);
    void write(uint32_t buffer);
    virtual void close() = 0;
};

#endif /* BYTE_STREAM_WRITER_H */
