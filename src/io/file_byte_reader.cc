#include <iostream>

#include <io/file_byte_reader.h>

FileByteReader::FileByteReader(std::string path)
{
    f.open(path.c_str(), std::ifstream::binary);
}

FileByteReader::~FileByteReader()
{
    f.close();
}

void FileByteReader::read(uint8_t *buffer, size_t count)
{
    f.read((char *) buffer, count);
}
