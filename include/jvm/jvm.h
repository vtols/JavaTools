#ifndef JVM_H
#define JVM_H

#include <class/java_class.h>

struct Class;
struct Method;
struct Frame;
class Interpreter;
class Thread;

struct Class
{
    ClassFile *classFile;

    Class(ClassFile *classFile);
    /* Doesn't take in account method signature */
    Method *getMethod(std::string name);

    static Class *loadClass(ByteReader *br);
    static Class *loadClass(ClassFile *cf);
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
    uint32_t pc;
    uint32_t *stack, *locals;
    uint16_t stackTop, maxLocals;
    uint8_t *code;
};

class Interpreter
{
public:
    Frame *current;

    void run();
    Frame *newFrame(Method *m);
    void pushFrame(Frame *f);
    void popFrame();
    void debugFrame();
};

class Thread
{
public:
    /* Now actually doesn't create new thread */
    /* And doesn't pass any arguments to method */
    void runThread(Method *m);

private:
    Frame *current;
};

#endif /* JVM_H */
