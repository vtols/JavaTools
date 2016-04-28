#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <string>
#include <fstream>

#include <char_stream.h>

class FileStream : public CharStream
{
public:
    FileStream(std::string path);
    virtual ~FileStream();

    wchar_t read();
    wchar_t peek();
    bool eof();

private:
    std::wifstream f;
    wchar_t c;
    bool end;

    void next();
};

#endif /* FILE_STREAM_H */
