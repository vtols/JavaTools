#include <iostream>

#include <io/file_byte_stream_writer.h>

FileByteStreamWriter::FileByteStreamWriter(std::string path)
{
    f.open(path.c_str(), std::ofstream::binary);
}

FileByteStreamWriter::~FileByteStreamWriter()
{
    f.close();
}

void FileByteStreamWriter::write(uint8_t *buffer, size_t count)
{
    f.write((char *) buffer, count);
    f.flush();
}

void FileByteStreamWriter::close()
{
    f.close();
}
