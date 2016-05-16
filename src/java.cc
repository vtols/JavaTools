#include <io/file_byte_reader.h>
#include <class/java_class.h>
#include <jvm/jvm.h>

int main(int argc, char *argv[])
{
    std::string classPath = argv[1];

    size_t found = classPath.find_last_of('.');
    std::string className = classPath.substr(0, found);

    Class *cls = ClassCache::getClass(className);
    Method *mainMethod =
            cls->getMethod("main", "([Ljava/lang/String;)V");

    Thread th;
    th.invoke(mainMethod);

    return 0;
}
