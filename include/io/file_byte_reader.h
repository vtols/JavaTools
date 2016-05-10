#ifndef FILE_BYTE_READER_H
#define FILE_BYTE_READER_H

#include <string>
#include <fstream>

#include <io/byte_reader.h>

class FileByteReader : public ByteReader
{
public:
    FileByteReader(std::string path);
    virtual ~FileByteReader();

    void read(uint8_t *buffer, size_t count);

private:
    std::ifstream f;
};

#endif /* FILE_BYTE_READER_H */
