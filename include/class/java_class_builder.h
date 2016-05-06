#ifndef JAVA_CLASS_BUILDER_H
#define JAVA_CLASS_BUILDER_H

#include <string>
#include <map>

#include <io/byte_stream_writer.h>
#include <io/byte_buffer.h>
#include <class/java_class.h>
#include <class/java_opcodes.h>

class ClassBuilder;
class MethodBuilder;
class Label;

class ClassBuilder
{
public:
    ClassBuilder(std::string name);
    MethodBuilder *createMethod(std::string name);
    uint16_t addUtf8(std::string str);
    uint16_t addClass(std::string name);
    uint16_t addString(std::string str);
    uint16_t addFieldRef(
        std::string className,
        std::string fieldName,
        std::string typeName);
    uint16_t addMethodRef(
        std::string className,
        std::string methodName,
        std::string typeName);
    void build(ByteStreamWriter *w);


private:
    std::string name;
    
    ClassFile classFile;
    
    std::vector<ConstantPoolInfo*> constantPool;
    uint16_t constCounter = 1;
    
    std::map<std::string, uint16_t> utf8Index;
    
    std::vector<MethodBuilder*> methodBuilders;

    uint16_t addNewItem(ConstantPoolInfo *ci);
    uint16_t addNameType(std::string name, std::string type);
};

class MethodBuilder
{
public:
    MethodBuilder(ClassBuilder *cb);
    void setAccessFlags(uint16_t flags);
    void setName(std::string name);
    void setDescriptor(std::string desc); 
    void setMax(uint16_t maxStack, uint16_t maxLocals);
    
    void insertLabel(Label *label);
    void instruction(uint8_t opCode);
    void field(
        uint8_t opCode,
        std::string className,
        std::string field,
        std::string descriptor);
    void invoke(
        uint8_t opCode,
        std::string className,
        std::string method,
        std::string descriptor);
    void loadString(std::string str);
    void jump(uint8_t opCode, Label *label);
    void frame();
    
    MemberInfo *build();

private:
    ClassBuilder *cb;
    uint16_t accessFlags, attributesCount;
    uint16_t maxStack, maxLocals;
    std::string name, descriptor;

    /* Now assume that all jump addresses are less than 2^16 */
    std::vector<uint8_t> code;
    ByteBuffer *codeBuilder;
    ByteStreamWriter *codeWriter;
    std::vector<Label*> labels;
    std::vector<Frame*> frames;
};

class Label
{
public:
    void addRef(uint32_t at);
    void setJumps(uint32_t addr, ByteBuffer *codeBuilder);

private:
    std::vector<uint32_t> refPositions;
};

struct FrameType
{
    uint8_t tag;
    std::string name;

    FrameType(uint8_t tag);
    FrameType(uint8_t tag, std::string name);
};

struct Frame
{
    uint8_t tag;
    uint16_t ref;
    std::vector<FrameType> locals, stack;
};

#endif /* JAVA_CLASS_BUILDER_H */
