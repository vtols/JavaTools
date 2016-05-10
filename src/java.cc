#include <io/file_byte_reader.h>
#include <class/java_class.h>
#include <jvm/jvm.h>

int main()
{
    FileByteReader f("Sample.class");

    Class *cls = Class::loadClass(&f);
    Method *mainMethod = cls->getMethod("main");

    Thread th;
    th.runThread(mainMethod);

    return 0;
}
