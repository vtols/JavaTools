#ifndef JVM_H
#define JVM_H

#include <map>
#include <stack>

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
    T_BOOLEAN = 4,
    T_CHAR    = 5,
    T_FLOAT   = 6,
    T_DOUBLE  = 7,
    T_BYTE    = 8,
    T_SHORT   = 9,
    T_INT     = 10,
    T_LONG    = 11,
    T_MAX     = 12;

const int
    BYTE_SIZE    = 1,
    CHAR_SIZE    = 2,
    DOUBLE_SIZE  = 8,
    FLOAT_SIZE   = 4,
    INTEGER_SIZE = 4,
    LONG_SIZE    = 8,
    OBJECT_SIZE  = sizeof(uintptr_t),
    SHORT_SIZE   = 2,
    BOOLEAN_SIZE = 1;

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

    Method *classInit = nullptr;
    bool initDone = false, initStarted = false;
    Thread *initThread = nullptr;

    uint16_t staticFieldsLength = 0, fieldsLength = 0;
    std::map<std::string, uint16_t> fieldOffset;
    std::map<std::string, std::string> descriptors;
    std::vector<std::string> fieldNames, staticFieldNames;
    uint8_t *staticFields;

    std::map<std::string, Method*> methods;

    static uint8_t fieldSize(std::string descriptor);

    Class(ClassFile *classFile);
    Object *newObject();
    Method *getMethod(std::string name, std::string descriptor);

protected:
    Class();
};

struct ArrayClass : Class
{
    bool arrayOfPrimitives;
    union {
        Class *classBase;
        uint8_t primitiveBase;
    } arrayBase;

    ArrayClass(Class *baseClass);
    ArrayClass(uint8_t basePrimitive);

    Object *newArray(int32_t length); // Array creation
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
    uint8_t fields[1];

    static Object *newObject(Class *cls);
    static Object *newArray(ArrayClass *cls, uint32_t length);

private:
    static Object *newObjectBlock(Class *cls, uint32_t size);
};

struct Method
{
    Class *owner;
    MemberInfo *methodInfo;
    CodeAttribute *codeAttr;
    uint32_t codeLength;
    uint8_t *code;

    std::vector<std::string> argDescriptors;
    std::string returnDescriptor;

    bool isInit = false;

    Method(Class *owner, MemberInfo *info);
};

struct Frame
{
    Frame *prev;
    Method *owner;
    /* PC will be used later */
    uint32_t pc;
    intptr_t *stack, *locals;
    uint16_t stackTop, maxStack, maxLocals;
    uint8_t *code;

    Frame(Method *m);
    ~Frame();
};

class Thread
{
public:
    void invoke(Method *m);

    void pushMethod(Method *m);
    void pushFrame(Frame *f);
    void popFrame();
    Frame *newFrame(Method *m);

    void prepareInit(Class *c);

    void runLoop();



private:
    std::stack<Method *> initStack;

    Frame *top = nullptr, *prev;
    uint32_t pc;
    uint8_t *code;
    intptr_t *locals, *stack;
    uint16_t stackTop;

    Class *frameClass, *memberClass;
    RefInfo *ref;
    std::string memberName, descriptor;
    bool isRef, isWide;
    uint16_t offset;
    uint8_t *fieldPtr;
    bool instanceMethod;
    Method *resolvedMethod;
    Object *tmpObject;
    uint32_t ret;

    void loadFrame();
    void saveFrame();

    void pushInit();
    bool prepareClass(bool ofMember);
    void prepareMember();
    bool prepareStaticField();
    void prepareField();
    bool prepareMethod();
    void selectOverriding();
    void loadField();
    void storeField();
    void loadArgs();
    void newArray(uint8_t type);
    template<typename T> T *arrayPointer(uint16_t stackOffset, int32_t index);
    void loadIntArray();
    void storeIntArray();
    void loadBoolArray();
    void storeBoolArray();
};

class Debug
{
public:
    static void debugCallStack(Frame *top);
    static void debugFrame(Frame *frame);
    static void debugObject(Object *obj, int depth=1);
    static void debugArrayObject(Object *array, int depth=1);
    static void debugIntArray(int32_t *ptr, int32_t length);
    static void debugBoolArray(int8_t *ptr, int32_t length);
};

#endif /* JVM_H */
