#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include <vector>

#include <io/byte_stream_writer.h>

class ByteBuffer : public ByteStreamWriter
{
public:
    ByteBuffer(std::vector<uint8_t> *bytes);
    virtual ~ByteBuffer();

    void setPosition(uint32_t position);
    void write(uint8_t *buffer, size_t count);
    void close();
    uint32_t written;

private:
    uint32_t position;
    std::vector<uint8_t> *bytes;
};

#endif /* BYTE_BUFFER_H */

