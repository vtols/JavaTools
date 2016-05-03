#ifndef JAVA_CLASS_H
#define JAVA_CLASS_H

#include <vector>
#include <string>

#include <byte_stream.h>
//#include <byte_stream_writer.h>

struct ConstantPoolInfo;
struct ClassFile;
struct ConstantPoolInfo;
struct RefInfo;
struct IndexInfo;
struct Utf8Info;
struct MemberInfo;
struct AttributeInfo;
struct Exception;
struct CodeAttribute;
struct LineNumber;
struct LineNumberTableAttribute;
struct StackMapFrame;
struct StackMapTableAttribute;
struct SameFrame;
struct SourceFileAttribute;

const uint8_t
    CONSTANT_Class = 7,
    CONSTANT_Fieldref = 9,
    CONSTANT_Methodref = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_String = 8,
    CONSTANT_Integer = 3,
    CONSTANT_Float = 4,
    CONSTANT_Long = 5,
    CONSTANT_Double = 6,
    CONSTANT_NameAndType = 12,
    CONSTANT_Utf8 = 1,
    CONSTANT_MethodHandle = 15,
    CONSTANT_MethodType = 16,
    CONSTANT_InvokeDynamic = 18;

struct ClassFile
{
    uint32_t magic;

    uint16_t minorVersion;
    uint16_t majorVersion;
    
    uint16_t constantPoolCount;
    std::vector<ConstantPoolInfo*> constantPool;

    uint16_t accessFlags;

    uint16_t thisClass;
    uint16_t superClass;

    uint16_t interfacesCount;
    std::vector<uint16_t> interfaces;

    uint16_t fieldsCount;
    std::vector<MemberInfo*> fields;

    uint16_t methodsCount;
    std::vector<MemberInfo*> methods;

    uint16_t attributesCount;
    std::vector<AttributeInfo*> attributes;

    static ClassFile read(ByteStream *bs);
    void write(ByteStream *bs);
//    static ClassFile void(ByteStreamWriter *bs);
    std::string getUtf8(uint16_t index);
};

struct ConstantPoolInfo
{
    uint8_t tag;

    static ConstantPoolInfo *read(ByteStream *bs);
};

struct RefInfo : ConstantPoolInfo
{
    uint16_t classIndex, nameAndTypeIndex;

    static RefInfo *read(ByteStream *bs);
};

struct IndexInfo : ConstantPoolInfo
{
    uint16_t index;

    static IndexInfo *read(ByteStream *bs);
};

struct Utf8Info : ConstantPoolInfo
{
    std::string str;

    static Utf8Info *read(ByteStream *bs);
};

struct MemberInfo
{
    uint16_t accessFlags;
    uint16_t nameIndex;
    uint16_t descriptorIndex;

    uint16_t attributesCount;
    std::vector<AttributeInfo*> attributes;

    static MemberInfo *read(ByteStream *bs, ClassFile *cf);
};

struct AttributeInfo
{
    uint16_t nameIndex;
    uint32_t length;

    static AttributeInfo *read(ByteStream *bs, ClassFile *cf);
};

struct Exception
{
    uint16_t startPc;
    uint16_t endPc;
    uint16_t handlerPc;
    uint16_t catchType;
    
    static Exception read(ByteStream *bs);
};

struct CodeAttribute : AttributeInfo
{
    uint16_t maxStack;
    uint16_t maxLocals;

    uint32_t codeLength;
    uint8_t *code;

    uint16_t exceptionTableLength;
    std::vector<Exception> exceptionTable;

    uint16_t attributesCount;
    std::vector<AttributeInfo*> attributes;

    static CodeAttribute *read(ByteStream *bs, ClassFile *cf);
};

struct LineNumber
{
    uint16_t startPc;
    uint16_t lineNumber;

    static LineNumber read(ByteStream *bs);
};

struct LineNumberTableAttribute : AttributeInfo
{
    uint16_t lineNumberTableLength;
    std::vector<LineNumber> lineNumberTable;

    static LineNumberTableAttribute *read(ByteStream *bs);
};

struct StackMapFrame
{
    uint8_t frameType;

    static StackMapFrame *read(ByteStream *bs);
};

struct StackMapTableAttribute : AttributeInfo
{
    uint16_t numberOfEntries;
    std::vector<StackMapFrame*> entries;

    static StackMapTableAttribute *read(ByteStream *bs);
};

struct SameFrame : StackMapFrame
{
    static SameFrame *read(ByteStream *bs);
};

struct SourceFileAttribute : AttributeInfo
{
    uint16_t sourceFileIndex;
    
    static SourceFileAttribute *read(ByteStream *bs);
};

#endif /* JAVA_CLASS_H */
