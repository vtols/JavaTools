#ifndef FILE_BYTE_WRITER_H
#define FILE_BYTE_WRITER_H

#include <string>
#include <fstream>

#include <io/byte_writer.h>

class FileByteWriter : public ByteWriter
{
public:
    FileByteWriter(std::string path);
    virtual ~FileByteWriter();

    void write(uint8_t *buffer, size_t count);
    void close();

private:
    std::ofstream f;
};

#endif /* FILE_BYTE_WRITER_H */

