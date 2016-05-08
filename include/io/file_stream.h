#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <string>
#include <fstream>

#include <io/char_stream.h>

class FileStream : public CharStream
{
public:
    FileStream(std::string path);
    virtual ~FileStream();

    char read();
    char peek();
    bool eof();

private:
    std::ifstream f;
    char c;
    bool end = false;

    void next();
};

#endif /* FILE_STREAM_H */
