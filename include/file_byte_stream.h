#ifndef FILE_BYTE_STREAM_H
#define FILE_BYTE_STREAM_H

#include <string>
#include <fstream>

#include <byte_stream.h>

class FileByteStream : public ByteStream
{
public:
    FileByteStream(std::string path);
    virtual ~FileByteStream();

    void read(uint8_t *buffer, size_t count);

private:
    std::ifstream f;
};

#endif /* FILE_BYTE_STREAM_H */
