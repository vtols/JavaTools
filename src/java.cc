#include <io/file_byte_reader.h>
#include <class/java_class.h>
#include <jvm/jvm.h>

int main()
{
    Class *cls = ClassLoader::loadClass("Sample");
    Method *mainMethod =
            cls->getMethod("main", "([Ljava/lang/String;)V");

    Thread th;
    th.invoke(mainMethod);

    return 0;
}
