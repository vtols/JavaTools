#ifndef CHAR_STREAM_H
#define CHAR_STREAM_H

class CharStream
{
public:
    virtual ~CharStream() = 0;
    
    virtual wchar_t read() = 0;
    virtual wchar_t peek() = 0;
    virtual bool eof() = 0;
};

#endif /* CHAR_STREAM_H */
