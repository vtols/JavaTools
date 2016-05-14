#ifndef JVM_H
#define JVM_H

#include <map>

#include <class/java_class.h>

class ClassLoader;
struct Class;
struct Method;
struct Frame;
class Interpreter;
class Thread;

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

    Class(ClassFile *classFile);
    Method *getMethod(std::string name, std::string descriptor);
};

class ClassCache
{
public:
    Class *getClass(std::string path);

private:
    /* Mapping from class name to class itself */
    std::map<std::string, Class*> classMap;
};

struct Object
{
    Class *cls;
    void *data;
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
