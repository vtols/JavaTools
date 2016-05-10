#include <iostream>

#include <io/file_byte_writer.h>

FileByteWriter::FileByteWriter(std::string path)
{
    f.open(path.c_str(), std::ofstream::binary);
}

FileByteWriter::~FileByteWriter()
{
    f.close();
}

void FileByteWriter::write(uint8_t *buffer, size_t count)
{
    f.write((char *) buffer, count);
    f.flush();
}

void FileByteWriter::close()
{
    f.close();
}
