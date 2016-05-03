#ifndef FILE_BYTE_STREAM_WRITER_H
#define FILE_BYTE_STREAM_WRITER_H

#include <string>
#include <fstream>

#include <byte_stream_writer.h>

class FileByteStreamWriter : public ByteStreamWriter
{
public:
    FileByteStreamWriter(std::string path);
    virtual ~FileByteStreamWriter();

    void write(uint8_t *buffer, size_t count);
    void close();

private:
    std::ofstream f;
};

#endif /* FILE_BYTE_STREAM_WRITER_H */

