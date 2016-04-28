#include <iostream>

#include <file_stream.h>

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
        c = L'\0';
        end = true;
    }
}

wchar_t FileStream::peek()
{
    return c;
}

wchar_t FileStream::read()
{
    wchar_t ret = c;
    next();
    return ret;
}

bool FileStream::eof()
{
    return end;
}
