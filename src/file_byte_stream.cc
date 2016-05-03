#include <iostream>

#include <file_byte_stream.h>

FileByteStream::FileByteStream(std::string path)
{
    f.open(path.c_str(), std::ifstream::binary);
}

FileByteStream::~FileByteStream()
{
    f.close();
}

void FileByteStream::read(uint8_t *buffer, size_t count)
{
    f.read((char *) buffer, count);
}
