#include <io/byte_reader.h>

ByteReader::~ByteReader()
{
}

uint8_t ByteReader::read8()
{
    uint8_t buffer[1];

    this->read(buffer, sizeof(buffer));
    return buffer[0];
}

uint16_t ByteReader::read16()
{
    uint8_t buffer[2];

    this->read(buffer, sizeof(buffer));
    return (uint16_t) buffer[0] << 8 |
           (uint16_t) buffer[1];
}

uint32_t ByteReader::read32()
{
    uint8_t buffer[4];

    this->read((uint8_t *) &buffer, sizeof(buffer));
    return (uint32_t) buffer[0] << 24 |
           (uint32_t) buffer[1] << 16 |
           (uint32_t) buffer[2] << 8  |
           (uint32_t) buffer[3];
}
