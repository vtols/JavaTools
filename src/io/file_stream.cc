#include <iostream>

#include <io/file_stream.h>

FileStream::FileStream(std::string path)
{
    f.open(path.c_str());
    next();
}

FileStream::~FileStream()
{
    f.close();
}

void FileStream::next()
{
    if (end || !f.get(c)) {
        c = '\0';
        end = true;
    }
}

char FileStream::peek()
{
    return c;
}

char FileStream::read()
{
    char ret = c;
    next();
    return ret;
}

bool FileStream::eof()
{
    return end;
}
