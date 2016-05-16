#ifndef JVM_H
#define JVM_H

#include <map>

#include <class/java_class.h>

class ClassLoader;
struct Class;
class ClassCache;
struct Object;
struct Method;
struct Frame;
class Interpreter;
class Thread;

const int
    BYTE_SIZE    = 1,
    CHAR_SIZE    = 2,
    DOUBLE_SIZE  = 8,
    FLOAT_SIZE   = 4,
    INTEGER_SIZE = 4,
    LONG_SIZE    = 8,
    OBJECT_SIZE  = sizeof(uintptr_t),
    SHORT_SIZE   = 2,
    BOOLEAN_SIZE = 1,
    ARRAY_SIZE   = sizeof(uintptr_t);

class ClassLoader
{
public:
    /* Now load only from current dir */
    static Class *loadClass(std::string path);

private:
    static Class *loadClass(ByteReader *br);
    static Class *loadClass(ClassFile *cf);
};

struct Class
{
    ClassFile *classFile;

    Class *super;

    uint16_t staticFieldsLength, fieldsLength;
    std::map<std::string, uint16_t> fieldOffset;
    uint8_t *staticFields;

    std::map<std::string, Method*> methods;

    Class(ClassFile *classFile);
    static uint8_t fieldSize(std::string descriptor);
    Object *newObject();
    Method *getMethod(std::string name, std::string descriptor);
};

class ClassCache
{
public:
    static Class *getClass(std::string path);

private:
    /* Mapping from class name to class itself */
    static std::map<std::string, Class*> classMap;
};

struct Object
{
    Class *cls;
    uint8_t *fields;

    Object(Class *cls);
};

struct Method
{
    Class *owner;
    MemberInfo *methodInfo;
    CodeAttribute *codeAttr;
    uint32_t codeLength;
    uint8_t *code;

    Method(Class *owner, MemberInfo *info);
};

struct Frame
{
    Frame *prev;
    Method *owner;
    /* PC will be used later */
    uint32_t pc;
    uint32_t *stack, *locals;
    uint16_t stackTop, maxLocals;
    uint8_t *code;

    void debug();
};

struct Stack
{
    Frame *top = nullptr;

    void pushMethod(Method *m);
    void pushFrame(Frame *f);
    void popFrame();
    Frame *newFrame(Method *m);
};

class Interpreter
{
public:
    Stack frameStack;

    void run();
};

class Thread
{
public:
    void invoke(Method *m);
};

#endif /* JVM_H */
