#include <io/byte_writer.h>

ByteWriter::~ByteWriter()
{
}

void ByteWriter::write(uint8_t buffer)
{
    this->write(&buffer, 1);
}

void ByteWriter::write(uint16_t buffer)
{
    uint8_t *buffer_ptr = (uint8_t *) &buffer;
    
    this->write(&buffer_ptr[1], 1);
    this->write(&buffer_ptr[0], 1);
}

void ByteWriter::write(uint32_t buffer)
{
    uint8_t *buffer_ptr = (uint8_t *) &buffer;
    
    this->write(&buffer_ptr[3], 1);
    this->write(&buffer_ptr[2], 1);
    this->write(&buffer_ptr[1], 1);
    this->write(&buffer_ptr[0], 1);
}
