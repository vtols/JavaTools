#ifndef BYTE_READER_H
#define BYTE_READER_H

#include <cstddef>
#include <cstdint>

class ByteReader
{
public:
    virtual ~ByteReader() = 0;
    
    virtual void read(uint8_t *buffer, size_t count) = 0;
    virtual uint8_t read8();
    virtual uint16_t read16();
    virtual uint32_t read32();
    //virtual bool eof();
};

#endif /* BYTE_READER_H */
