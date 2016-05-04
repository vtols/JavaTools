#include <vector>
#include <algorithm>

#include <io/byte_buffer.h>

ByteBuffer::ByteBuffer(std::vector<uint8_t> *bytes)
{
    this->bytes = bytes;
    written = 0;
    position = 0;
}

ByteBuffer::~ByteBuffer()
{
}

void ByteBuffer::setPosition(uint32_t position)
{
    this->position = position;
}

void ByteBuffer::write(uint8_t* buffer, size_t count)
{
    auto vecPos = bytes->begin() + position;
    if (vecPos != bytes->end()) {
        uint32_t eraseCount = count;
        if (bytes->end() - vecPos < eraseCount)
            eraseCount = bytes->end() - vecPos;
        bytes->erase(vecPos, vecPos + eraseCount);
        written -= eraseCount;
    }
    bytes->insert(vecPos, buffer, buffer + count);
    written += count;
    position += count;
}

void ByteBuffer::close()
{
}
