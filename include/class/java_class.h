#ifndef JAVA_CLASS_H
#define JAVA_CLASS_H

#include <vector>
#include <string>

#include <io/byte_reader.h>
#include <io/byte_writer.h>

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

const uint16_t
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_SUPER = 0x0020,
    ACC_VOLATILE = 0x0040,
    ACC_TRANSIENT = 0x0080,
    ACC_INTERFACE = 0x0200,
    ACC_ABSTRACT = 0x0400,
    ACC_SYNTHETIC = 0x1000,
    ACC_ANNOTATION = 0x2000,
    ACC_ENUM = 0x4000,
    ACC_SYNCHRONIZED = 0x0020,
    ACC_BRIDGE = 0x0040,
    ACC_VARARGS = 0x0080,
    ACC_NATIVE = 0x0100,
    ACC_STRICT = 0x0800;

const uint8_t
    ITEM_Top = 0,
    ITEM_Integer = 1,
    ITEM_Float = 2,
    ITEM_Double = 3,
    ITEM_Long = 4,
    ITEM_Null = 5,
    ITEM_UninitializedThis = 6,
    ITEM_Object = 7,
    ITEM_Uninitialized = 8;


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

    static ClassFile read(ByteReader *bs);
    void write(ByteWriter *bs);
    std::string getUtf8(uint16_t index);
    std::string getIndexName(uint16_t index);
};

struct ConstantPoolInfo
{
    uint8_t tag;

    static ConstantPoolInfo *read(ByteReader *bs);
    virtual void write(ByteWriter *bs);
};

struct RefInfo : ConstantPoolInfo
{
    uint16_t firstIndex, secondIndex;

    static RefInfo *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct IndexInfo : ConstantPoolInfo
{
    uint16_t index;

    static IndexInfo *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct Const32Info : ConstantPoolInfo
{
    uint32_t value;

    static Const32Info *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct Utf8Info : ConstantPoolInfo
{
    std::string str;

    static Utf8Info *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct MemberInfo
{
    uint16_t accessFlags;
    uint16_t nameIndex;
    uint16_t descriptorIndex;

    uint16_t attributesCount;
    std::vector<AttributeInfo*> attributes;

    static MemberInfo *read(ByteReader *bs, ClassFile *cf);
    void write(ByteWriter *bs);
};

struct AttributeInfo
{
    uint16_t nameIndex;
    uint32_t length;

    static AttributeInfo *read(ByteReader *bs, ClassFile *cf);
    virtual void write(ByteWriter *bs);
};

struct Exception
{
    uint16_t startPc;
    uint16_t endPc;
    uint16_t handlerPc;
    uint16_t catchType;
    
    static Exception read(ByteReader *bs);
    void write(ByteWriter *bs);
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

    static CodeAttribute *read(ByteReader *bs, ClassFile *cf);
    void write(ByteWriter *bs);
};

struct LineNumber
{
    uint16_t startPc;
    uint16_t lineNumber;

    static LineNumber read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct LineNumberTableAttribute : AttributeInfo
{
    uint16_t lineNumberTableLength;
    std::vector<LineNumber> lineNumberTable;

    static LineNumberTableAttribute *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct VerificationTypeInfo
{
    uint8_t tag;
    uint16_t data;

    static VerificationTypeInfo read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct StackMapFrame
{
    uint8_t frameType;

    uint16_t frameDelta;

    uint16_t numberLocals;
    std::vector<VerificationTypeInfo> locals;

    uint16_t numberStack;
    std::vector<VerificationTypeInfo> stack;

    static StackMapFrame *read(ByteReader *bs);
    virtual void write(ByteWriter *bs);
};

struct StackMapTableAttribute : AttributeInfo
{
    uint16_t numberOfEntries;
    std::vector<StackMapFrame*> entries;

    static StackMapTableAttribute *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct SourceFileAttribute : AttributeInfo
{
    uint16_t sourceFileIndex;

    static SourceFileAttribute *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct Variable
{
    uint16_t startPc;
    uint16_t length;
    uint16_t nameIndex;
    uint16_t signatureIndex;
    uint16_t index;

    static Variable read(ByteReader *bs);
    void write(ByteWriter *bs);
};

struct LocalVariableTableAttribute : AttributeInfo
{
    uint16_t numberOfEntries;
    std::vector<Variable> entries;

    static LocalVariableTableAttribute *read(ByteReader *bs);
    void write(ByteWriter *bs);
};

#endif /* JAVA_CLASS_H */
